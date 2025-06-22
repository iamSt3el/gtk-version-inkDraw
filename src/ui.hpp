#pragma once

#include <gtkmm.h>
#include <functional>
#include "canvas.hpp"


struct ToolInfo{
    std::string name;
    std::string icon_name;
    std::string tooltip;
    std::string css_class;
};

class UI_ToolBar : public Gtk::ApplicationWindow{
    public:
        UI_ToolBar();
        virtual ~UI_ToolBar() = default;
        
        // Set callback for tool changes
        void set_tool_change_callback(std::function<void(const std::string&, CairoDrawingArea*)> callback);

    private:
        // Current tool
        std::string current_tool_name;
        
        // Callback for tool changes
        std::function<void(const std::string&, CairoDrawingArea*)> tool_change_callback;
        void setup_css();
        void create_toolbar();
        Gtk::Button* create_tool_button(const ToolInfo& tool);
        Gtk::Separator* create_separator();

        // Event handlers
        void on_tool_selected(Gtk::Button* button, const std::string& tool_name);

        // Member variables
        std::vector<ToolInfo> tools;
        std::vector<Gtk::Button*> tool_buttons;
        Gtk::Button* selected_button;
        std::string get_current_tool();
        Canvas canvas;

        // UI widgets
        Gtk::Box main_box;
        Gtk::CenterBox toolbar_wrapper;
        Gtk::Box toolbar_container;
};
