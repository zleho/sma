#include "Application.h"

static inline const char* toString(AppState state)
{
    static const char* strings[static_cast<unsigned>(AppState::stateNum)] = {
        "Connecting to PulseAudio.",
        "Getting device list.",
        "Configuration.",
        "Connecting stream.",
        "Taking measurement.",
        "Disconnecting stream",
        "Disconnecting from Pulse Audio",
        "PulseAudio Connection terminated"
    };

    return strings[static_cast<unsigned>(state)];
}

static void pulseStateCb(pa_context*, void*);
static void pulseDevicesCb(pa_context*, const pa_source_info*, int, void*);
static void pulseStreamStateCb(pa_stream*, void*);
static void pulseStreamOverflowCb(pa_stream*, void*);
static void pulseStreamReadCb(pa_stream*, size_t, void*);

AppWindow::AppWindow() 
    : overflowNum_(0), 
      rms_("RMS (dB)"), 
      itu_("ITU BS-1770 (dB)"),
      third_("Weigthed third-octave (dB)")
{
    pulseLoop_ = pa_glib_mainloop_new(nullptr);
    pulseApi_ = pa_glib_mainloop_get_api(pulseLoop_);
    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "Sound Measurement Application");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.sma.SoundMeasurementApplication");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "0.1");
    pulseCtx_ = pa_context_new_with_proplist(pulseApi_, nullptr, proplist);
    pa_proplist_free(proplist);
    pa_context_set_state_callback(pulseCtx_, pulseStateCb, this);
    pulseSample_.format = PA_SAMPLE_S16LE;
    pulseSample_.rate = 48000;
    pulseSample_.channels = 1;
    pulseStream_ = nullptr;

    set_title("Sound Measurement Application 0.1");
    set_default_size(300, -1);
    
    measButton_.set_label("MEASURE");
    measButton_.signal_clicked().connect(sigc::mem_fun(*this, &AppWindow::onMeasButton));

    box_.add(config_);
    box_.add(measButton_);
    box_.add(rms_);
    box_.add(itu_);
    box_.add(third_);
    box_.add(statusBar_);
    add(box_);
    show_all_children();
    
    setState(AppState::conn);
}

AppWindow::~AppWindow()
{
    if (pulseStream_) {
        switch (pa_stream_get_state(pulseStream_)) {
        case PA_STREAM_CREATING:
        case PA_STREAM_READY:
            pa_stream_disconnect(pulseStream_);
            break;
        default:
            break;
        }
        pa_stream_unref(pulseStream_);
    }

    switch (pa_context_get_state(pulseCtx_)) {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
    case PA_CONTEXT_READY:
        pa_context_disconnect(pulseCtx_);
        break;
    default:
        break;
    }

    pa_context_unref(pulseCtx_);
    pa_glib_mainloop_free(pulseLoop_);
}

void AppWindow::setState(AppState state)
{
    switch (state) {
    case AppState::term:
        close();
        return;
    case AppState::conn:
        config_.set_sensitive(false);
        measButton_.set_sensitive(false);
        rms_.set_sensitive(false);
        itu_.set_sensitive(false);
        third_.set_sensitive(false);
        pa_context_connect(pulseCtx_, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
        break;
    case AppState::devices:
    {
        config_.clearDevices();
        auto op = pa_context_get_source_info_list(pulseCtx_, pulseDevicesCb, this);
        pa_operation_unref(op);
        break;
    }
    case AppState::config:
    {
        config_.set_sensitive(true);
        measButton_.set_sensitive(true);
        rms_.set_sensitive(false);
        itu_.set_sensitive(false);
        third_.set_sensitive(false);
        rms_.setValue(0.0);
        itu_.setValue(0.0);
        third_.setValue(0.0);
        break;
    }
    case AppState::connstream:
    {
        config_.set_sensitive(false);
        measButton_.set_sensitive(false);
        pulseStream_ = pa_stream_new(pulseCtx_, "SMA Record stream", &pulseSample_, nullptr);
        pa_stream_set_state_callback(pulseStream_, pulseStreamStateCb, this);
        pa_stream_set_overflow_callback(pulseStream_, pulseStreamOverflowCb, this);
        pa_stream_set_read_callback(pulseStream_, pulseStreamReadCb, this);
        pa_stream_connect_record(pulseStream_, config_.getActiveDevice().c_str(), nullptr, PA_STREAM_NOFLAGS);
        break;
    }
    case AppState::meas:
    {
        overflowNum_ = 0;
        rms_.init(config_.getSampleSize());
        rms_.set_sensitive(true);
        itu_.init(config_.getSampleSize());
        itu_.set_sensitive(true);
        third_.init(config_.getSampleSize());
        third_.set_sensitive(true);
        measButton_.set_sensitive(true);
        break;
    }
    case AppState::disconnstream:
    {
        rms_.set_sensitive(false);
        itu_.set_sensitive(false);
        third_.set_sensitive(false);
        config_.set_sensitive(false);
        measButton_.set_sensitive(false);
        pa_stream_disconnect(pulseStream_);
        pa_stream_unref(pulseStream_);
        pulseStream_ = nullptr;
        break;
    }
    default:
        break;
    }

    state_ = state;
    setStatusBar(toString(state_));
}

static void pulseStateCb(pa_context* pulseCtx, void* userdata)
{
    auto appWindow = static_cast<AppWindow*>(userdata);
    
    auto pulseState = pa_context_get_state(pulseCtx);
    switch (pulseState) {
    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        appWindow->setState(AppState::term);
        break;
    case PA_CONTEXT_READY:
        appWindow->setState(AppState::devices);
        break;
    default:
        break;
    }
}

static void pulseDevicesCb(pa_context*, const pa_source_info* src, int eol, void* userdata)
{
    auto appWindow = static_cast<AppWindow*>(userdata);
    
    if (eol < 0) {
        abort();
        // TODO: error reporting
        appWindow->setState(AppState::term);
        return;
    }
    
    if (eol > 0) {
        appWindow->setState(AppState::config);
        return;
    }

    appWindow->getConfig().addSource(src->name);
}

static void pulseStreamStateCb(pa_stream* stream, void* userdata)
{
    auto appWindow = static_cast<AppWindow*>(userdata);
    switch (pa_stream_get_state(stream)) {
    case PA_STREAM_TERMINATED:
        appWindow->setState(AppState::devices);
        break;
    case PA_STREAM_READY:
        appWindow->setState(AppState::meas);
        break;
    case PA_STREAM_FAILED:
        abort();
        // TODO: error reporting
        appWindow->setState(AppState::term);
        break;
    default:
        break;
    }
}

static void pulseStreamOverflowCb(pa_stream*, void* userdata)
{
    auto appWindow = static_cast<AppWindow*>(userdata);
    appWindow->incOverflow();
}

static void pulseStreamReadCb(pa_stream* stream, size_t nbytes, void* userdata)
{
    auto appWindow = static_cast<AppWindow*>(userdata);
    const void* data = nullptr;

    pa_stream_peek(stream, &data, &nbytes);
    while (data || nbytes) {
        if (data) {
            nbytes >>= 1;
        
            auto buffer = static_cast<const short*>(data); 
            while (nbytes--)
                appWindow->measure(*buffer++);
            
            pa_stream_drop(stream);
        }
        else if (nbytes)
            pa_stream_drop(stream);
        
        pa_stream_peek(stream, &data, &nbytes);
    }
}

void AppWindow::measure(int input)
{
    fixie::Fixed<long long, 16> x(input << 1, false);
    rms_.measure(x);
    itu_.measure(x);
    third_.measure(x);
}


