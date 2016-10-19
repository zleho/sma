#pragma once

#include <string>
#include <gtkmm/frame.h>
#include <gtkmm/hvbox.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>

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
        box_.set_margin_bottom(10);
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
        label_.set_text(std::to_string(val));
    }
private:
    MeasType meas_;
    Gtk::VBox box_;
    Gtk::Label label_;
    Gtk::ProgressBar bar_;
};


