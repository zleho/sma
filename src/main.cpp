#include <vector>
#include <string>
#include <iostream>
#include <cmath>

#include <gtkmm/window.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/application.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/frame.h>

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

#include <cassert>

enum class AppState : unsigned {
    conn,
    devices,
    config,
    connstream,
    meas,
    disconnstream,
    disconn,
    term,
    stateNum
};

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


static inline long long log2_floor(long long x)
{
    return 64 - 1 - 16 - __builtin_clzl(x);
}

/*
    x = 2^log2_floor(x)+y 
      = 2^log2_floor(x) * ((2^log2_floor(x)+y) / 2^log2_floor(x))
      = 2^log2_floor(x) * (x / 2^log2_floor(x))

    log2(x) = log2_floor(x) + log2(x / 2^log2_floor(x))

    1 <= x / 2^log2_floor(x) <= 2 -> 0 <= log2(x / 2^log2_floor(x)) <= 1
*/

static inline long long log2_fix(long long x)
{
    return log2_floor(x) << 16;
}

/*
   20*log10(RMS/(2^(-16))
    = 20*16*log10(2) + 20*log10(RMS)
    = C1 + 10*log10(SUM/N), where C1 = 20*16*log10(2)
    = C1 + -10*log10(N) + 10*log10(SUM)
    = C1 + C2 + (10/log2(10))*log2(SUM), where C2 = -10*log10(N)
    = C1 + C2 + C3 * log2(SUM), where C3 = 10/log2(10)

*/
class RMSdB {
public:
    explicit RMSdB(size_t n) 
        : size_(n),
          c1(6313056),
          c2(-10 * std::log10(size_)*65536),
          c3(197283)
    {
        init();
    }

    void init()
    {
        sum_ = 0;
        curr_ = 0;
    }

    bool step(long long x, int& ret)
    {
        x <<= 1;
        x *= x;
        x >>= 16;
        sum_ += x;

        if (++curr_ == size_) {
            sum_ = log2_fix(sum_) << 16;
            sum_ *= c3;
            sum_ += c1 + c2;
            ret = sum_ >> 16;
            init();
            return true;
        }
        return false;
    }
private:
    size_t curr_;
    size_t size_;
    long long sum_;
    long long c1, c2, c3;
};

class AppWindow : public Gtk::Window {
public:
    AppWindow();
    ~AppWindow();

    void setState(AppState);
    void addSource(const char*);
    void setStatusBar(const std::string& status)
    {
        statusBar_.remove_all_messages();
        statusBar_.push(status);
    }

    void incOverflow()
    {
        static const char* message = "Number of overflows: ";
        setStatusBar(message + std::to_string(++overflowNum_));
    }

    void measure(int);
private:
    void onMeasButton()
    {
        if (measButton_.get_active())
            setState(AppState::connstream);
        else
            setState(AppState::disconnstream);
    }

    AppState state_;
    bool firstDevice_;
    size_t overflowNum_;
    RMSdB rmsMeter_;

    pa_glib_mainloop* pulseLoop_;
    pa_mainloop_api* pulseApi_;
    pa_context* pulseCtx_;
    pa_stream* pulseStream_;
    pa_sample_spec pulseSample_;

    Gtk::Frame config_;
    Gtk::Label deviceLabel_;
    Gtk::ComboBoxText device_;
    Gtk::ToggleButton measButton_;
    Gtk::Frame measurements_;
    Gtk::Label rmsName_;
    Gtk::Label rmsValue_;
    Gtk::ProgressBar rmsBar_;
    Gtk::Statusbar statusBar_;
    Gtk::VBox rmsBox_;
    Gtk::HBox rmsLiterals_;

    Gtk::VBox box_;
};

AppWindow::AppWindow() : overflowNum_(0), rmsMeter_(19200)
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
    
    config_.set_label("Parameters");
    config_.add(device_);
    
    measButton_.set_label("MEASURE");
    measButton_.signal_clicked().connect(sigc::mem_fun(*this, &AppWindow::onMeasButton));

    measurements_.set_label("Measurements");
    rmsName_.set_text("RMS");
    rmsName_.set_halign(Gtk::ALIGN_START);
    rmsValue_.set_text("0 dB");
    rmsValue_.set_halign(Gtk::ALIGN_END);
    rmsLiterals_.add(rmsName_);
    rmsLiterals_.add(rmsValue_);
    rmsBox_.add(rmsLiterals_);
    rmsBox_.add(rmsBar_);
    measurements_.add(rmsBox_);

    box_.add(config_);
    box_.add(measButton_);
    box_.add(measurements_);
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

void AppWindow::addSource(const char* str)
{
    device_.append(str);
    if (firstDevice_) {
        device_.set_active_text(str);
        firstDevice_ = false;
    }
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
        measurements_.set_sensitive(false);
        pa_context_connect(pulseCtx_, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
        break;
    case AppState::devices:
    {
        device_.remove_all();
        firstDevice_ = true;
        auto op = pa_context_get_source_info_list(pulseCtx_, pulseDevicesCb, this);
        pa_operation_unref(op);
        break;
    }
    case AppState::config:
    {
        config_.set_sensitive(true);
        measButton_.set_sensitive(true);
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
        pa_stream_connect_record(pulseStream_, device_.get_active_text().c_str(), nullptr, PA_STREAM_NOFLAGS);
        break;
    }
    case AppState::meas:
    {
        overflowNum_ = 0;
        measurements_.set_sensitive(true);
        measButton_.set_sensitive(true);
        break;
    }
    case AppState::disconnstream:
    {
        measurements_.set_sensitive(false);
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
        // TODO: error reporting
        appWindow->setState(AppState::term);
        return;
    }
    
    if (eol > 0) {
        appWindow->setState(AppState::config);
        return;
    }

    appWindow->addSource(src->name);
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
        }    
        
        if (nbytes)
            pa_stream_drop(stream);
        
        pa_stream_peek(stream, &data, &nbytes);
    }
}

void AppWindow::measure(int x)
{
    int val;
    if (rmsMeter_.step(x, val)) {
        rmsValue_.set_text(std::to_string(val) + " dB");
        rmsBar_.set_fraction(val / 96.0);
    }
    
}

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.sma.SoundMeasurementApplication");
    
    AppWindow appWindow;
    return app->run(appWindow);
}

