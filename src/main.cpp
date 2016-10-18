#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <iomanip>

#include <gtkmm/window.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/application.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/frame.h>
#include <gtkmm/scale.h>

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

#include <cassert>

#include "fixie.h"

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


/*
   20*log10(RMS/(2^(-Q))
    = 20*Q*log10(2) + 20*log10(RMS)
    = C1 + 10*log10(SUM/N), where C1 = 20*16*log10(2)
    = C1 + -10*log10(N) + 10*log10(SUM)
    = C1 + C2 + (10/log2(10))*log2(SUM), where C2 = -10*log10(N)
    = C1 + C2 + C3 * log2(SUM), where C3 = 10/log2(10)

*/

template <class Fixed>
class RMSdB {
public:
    static constexpr double max() 
    {
        return 20*FixedType::power()*std::log10(2);
    }

    using FixedType = Fixed;

    RMSdB() {}

    explicit RMSdB(size_t n) 
        : size_(n),
          c1(max()),
          c2(-10*std::log10(size_)),
          c3(10/std::log2(10))
    {
        init();
    }

    void init()
    {
        sum_ = Fixed();
        curr_ = 0;
    }

    bool step(Fixed x, double& ret)
    {
        x *= x;
        sum_ += x;

        if (++curr_ == size_) {
            ret = 0.0;
            if (sum_.repr) {
                sum_ = fixie::log2(sum_);
                sum_ *= c3;
                sum_ += c1 + c2;
                ret = static_cast<double>(sum_);
            }
            init();
            return true;
        }
        return false;
    }
private:
    size_t curr_;
    size_t size_;
    Fixed sum_;
    Fixed c1, c2, c3;
};

template <class Fixed>
class BiQuad {
public:
    BiQuad()
    {
    }

    BiQuad(Fixed b0, Fixed b1, Fixed b2, Fixed a1, Fixed a2)
        : b0_(b0), b1_(b1), b2_(b2), a1_(a1), a2_(a2)
    {
        init();
    }

    void init()
    {
        w_ = w1_ = w2_ = Fixed(0);
    }

    Fixed operator()(Fixed x)
    {
        w_ = x - a1_*w1_ - a2_*w2_;
        auto y = b0_*w_ + b1_*w1_ + b2_*w2_;
        w2_ = w1_;
        w1_ = w_;
        return y;
    }
private:
    Fixed b0_, b1_, b2_;
    Fixed a1_, a2_;
    Fixed w_, w1_, w2_;
};

template <class Fixed>
class ITUBS1770 {
public:
    using FixedType = Fixed;
    static constexpr double max()
    {
        return RMSdB<Fixed>::max();
    }

    ITUBS1770()
    {
    }

    explicit ITUBS1770(std::size_t size)
        : stage1_(
            Fixed(1.53512485958697),
            Fixed(-2.69169618940638),
            Fixed(1.19839281085285),
            Fixed(-1.69065929318241),
            Fixed(0.73248077421585)
          ),
          stage2_(
            Fixed(1),
            Fixed(-2),
            Fixed(1),
            Fixed(-1.99004745483398),
            Fixed(0.99007225036621)
          ),
          rms_(size)
    {
    }

    bool step(Fixed x, double& val)
    {
        if (rms_.step(stage2_(stage1_(x)), val)) {
            stage1_.init();
            stage2_.init();
            return true;
        }

        return false;
    }
private:
    BiQuad<Fixed> stage1_;
    BiQuad<Fixed> stage2_;
    RMSdB<Fixed> rms_;
};

class Config : public Gtk::Frame {
public:
    Config();
    ~Config() override;
    void addSource(const char*);
    void clearDevices();
    std::string getActiveDevice();
    size_t getSampleSize() { return 48000 * measSize_.get_value(); }
private:
    bool firstDevice_;
    Gtk::Label deviceLabel_;
    Gtk::ComboBoxText device_;
    Gtk::Scale measSize_;
    Gtk::Frame deviceFrame_;
    Gtk::Frame measSizeFrame_;
    Gtk::VBox box_;
};

Config::Config()
    : firstDevice_(false)
{
    set_label("Parameters");
    
    deviceFrame_.set_label("Input device");
    deviceFrame_.add(device_);
    box_.add(deviceFrame_);
    
    measSize_.set_range(0.1, 1.0);
    measSize_.set_increments(0.1, 0.1);
    measSizeFrame_.set_label("Measurement size (s)");
    measSizeFrame_.add(measSize_);
    box_.add(measSizeFrame_);
    
    add(box_);
 }

Config::~Config()
{
}

void Config::addSource(const char* str)
{
    device_.append(str);
    if (firstDevice_) {
        device_.set_active_text(str);
        firstDevice_ = false;
    }
}

void Config::clearDevices()
{
    device_.remove_all();
    firstDevice_ = true;
}

std::string Config::getActiveDevice()
{
    return device_.get_active_text().c_str();
}

static inline std::string toString(double v)
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}

template <class MeasType>
class Measurement : public Gtk::Frame {
public:
    Measurement(const std::string& label)
    {
        set_label(label);
        label_.set_halign(Gtk::Align::ALIGN_END);
        box_.add(label_);
        box_.add(bar_);
        box_.set_margin_start(10);
        box_.set_margin_end(10);
        add(box_);
        setValue(0.0);
    }

    ~Measurement() override
    {
    }

    void init(size_t size)
    {
        meas_ = MeasType(size);
    }

    void measure(typename MeasType::FixedType x)
    {
        double value;
        if (meas_.step(x, value))
            setValue(value);
    }

    void setValue(double val)
    {
        bar_.set_fraction(val / MeasType::max());
        label_.set_text(toString(val));
    }
private:
    MeasType meas_;
    Gtk::VBox box_;
    Gtk::Label label_;
    Gtk::ProgressBar bar_;
};

using fix16ll = fixie::Fixed<long long, 16>;

class AppWindow : public Gtk::Window {
public:
    AppWindow();
    ~AppWindow() override;

    void setState(AppState);
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

    Config& getConfig() { return config_; }
private:
    void onMeasButton()
    {
        if (measButton_.get_active())
            setState(AppState::connstream);
        else
            setState(AppState::disconnstream);
    }

    AppState state_;
    size_t overflowNum_;

    pa_glib_mainloop* pulseLoop_;
    pa_mainloop_api* pulseApi_;
    pa_context* pulseCtx_;
    pa_stream* pulseStream_;
    pa_sample_spec pulseSample_;

    Config config_;
    Gtk::ToggleButton measButton_;
    Measurement<RMSdB<fix16ll>> rms_;
    Measurement<ITUBS1770<fix16ll>> itu_;
    Gtk::Statusbar statusBar_;
    Gtk::VBox box_;
};

AppWindow::AppWindow() : overflowNum_(0), rms_("RMS (dB)"), itu_("ITU BS-1770 (dB)")
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
        rms_.setValue(0.0);
        itu_.setValue(0.0);
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
        measButton_.set_sensitive(true);
        break;
    }
    case AppState::disconnstream:
    {
        rms_.set_sensitive(false);
        itu_.set_sensitive(false);
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
    fix16ll x(input << 1, false);
    rms_.measure(x);
    itu_.measure(x);
}

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.sma.SoundMeasurementApplication");
    
    AppWindow appWindow;
    return app->run(appWindow);
}

