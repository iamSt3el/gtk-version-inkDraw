#include <gtkmm.h>
#include "ui.hpp"
#include "drawingLogic.hpp"
#include <memory>
#include <vector>
#include <iostream>

class App : public Gtk::Application{
    public:
        static Glib::RefPtr<App> create(){
            return Glib::make_refptr_for_instance<App>(
                    new App());
        }

    protected:
        App() : Gtk::Application("com.example.App") {}

        void on_activate() override{
            auto window = std::make_unique<UI_ToolBar>();
            
            // Connect the tool change callback
            window->set_tool_change_callback(on_tool_changed);
            
            add_window(*window);
            window->show();

            windows.push_back(std::move(window));
        }

    private:
        std::vector<std::unique_ptr<UI_ToolBar>> windows;
    
};


int main(int argc, char** argv){
    auto app = App::create();
    return app->run(argc, argv);
}
