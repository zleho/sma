#include "Application.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.sma.SoundMeasurementApplication");
    
    AppWindow appWindow;
    return app->run(appWindow);
}

