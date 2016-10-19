#include "Config.h"

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

std::size_t Config::getSampleSize() 
{ 
    return 48000 * measSize_.get_value(); 
}

