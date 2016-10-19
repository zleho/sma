#pragma once

#include <string>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/scale.h>
#include <gtkmm/hvbox.h>

class Config : public Gtk::Frame {
public:
    Config();
    ~Config() override;
    void addSource(const char*);
    void clearDevices();
    std::string getActiveDevice();
    std::size_t getSampleSize();
private:
    bool firstDevice_;
    Gtk::Label deviceLabel_;
    Gtk::ComboBoxText device_;
    Gtk::Scale measSize_;
    Gtk::Frame deviceFrame_;
    Gtk::Frame measSizeFrame_;
    Gtk::VBox box_;
};

