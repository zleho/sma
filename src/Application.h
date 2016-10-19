#pragma once

#include <gtkmm/window.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/frame.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

#include "Config.h"
#include "Measurement.h"
#include "fixie.h"
#include "RMSdB.h"
#include "ITUBS1770.h"

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
    Measurement<RMSdB<fixie::Fixed<long long, 16>>> rms_;
    Measurement<ITUBS1770<fixie::Fixed<long long, 16>>> itu_;
    Gtk::Statusbar statusBar_;
    Gtk::VBox box_;
};

