#include "ui.hpp"
#include "gtkmm/enums.h"
#include <gtkmm.h>
#include <iostream>


// CSS styles (same as C version)
const char* css_data = R"(
window {
  background: #f8f9fa;
}

.toolbar-wrapper {
  padding: 20px;
}

.toolbar-container {
  background: rgba(255, 255, 255, 0.95);
  border-radius: 12px;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.1),
             0 2px 8px rgba(0, 0, 0, 0.05);
  padding: 8px 12px;
  border: 1px solid rgba(0, 0, 0, 0.06);
  backdrop-filter: blur(10px);
}

.tool-button {
  color: black;
  background:transparent;
  border: 2px solid transparent;
  border-radius: 8px;
  padding: 8px;
  margin: 2px;
  transition: all 150ms cubic-bezier(0.4, 0, 0.2, 1);
  min-width: 20px;
  min-height: 20px;
}

.tool-button:hover {
  background: rgba(99, 102, 241, 0.1);
  border-color: rgba(99, 102, 241, 0.2);
  transform: translateY(-1px);
}

.tool-button.selected {
  background: #6366f1;
  border-color: #4f46e5;
  color: white;
  box-shadow: 0 4px 12px rgba(99, 102, 241, 0.3);
}

.tool-button.selected:hover {
  background: #4f46e5;
  transform: translateY(-1px);
}

.tool-separator {
  background: rgba(0, 0, 0, 0.1);
  min-width: 1px;
  margin: 8px 4px;
}

.canvas_box{
    background-color: white;
}

.notebook_page{
    background-color: white;
    margin: 20px;
    padding: 30px;
    border: 1px solid #ddd;
    border-radius: 10px;
    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
}

.notebook_page.lined{
    background-image: linear-gradient(to bottom, transparent 24px, #ddd 24px, #ddd 25px, transparent 25px);
    background-size: 100% 25px;
    background-repeat: repeat;
}

.notebook_page.grid{
    background-image: 
        linear-gradient(to right, #ddd 1px, transparent 1px),
        linear-gradient(to bottom, #ddd 1px, transparent 1px);
    background-size: 20px 20px;
}

.notebook_page.dotted{
    background-image: radial-gradient(circle, #ddd 1px, transparent 1px);
    background-size: 15px 15px;
}

.notebook_page.graph{
    background-image: 
        linear-gradient(to right, #eee 1px, transparent 1px),
        linear-gradient(to bottom, #eee 1px, transparent 1px),
        linear-gradient(to right, #ddd 1px, transparent 1px),
        linear-gradient(to bottom, #ddd 1px, transparent 1px);
    background-size: 10px 10px, 10px 10px, 50px 50px, 50px 50px;
}

/* Tool-specific colors */
.select-tool.selected { background: #10b981; border-color: #059669; }
.rectangle-tool.selected { background: #f59e0b; border-color: #d97706; }
.circle-tool.selected { background: #ef4444; border-color: #dc2626; }
.arrow-tool.selected { background: #8b5cf6; border-color: #7c3aed; }
.line-tool.selected { background: #06b6d4; border-color: #0891b2; }
.pen-tool.selected { background: #ec4899; border-color: #db2777; }
.text-tool.selected { background: #6366f1; border-color: #4f46e5; }
.eraser-tool.selected { background: #6b7280; border-color: #4b5563; }
)";

// Constructor
UI_ToolBar::UI_ToolBar()
    : main_box(Gtk::Orientation::VERTICAL, 0),
      toolbar_container(Gtk::Orientation::HORIZONTAL, 2),
      canvas(Gtk::Orientation::VERTICAL, 0),
      selected_button(nullptr),
      current_tool_name("pen")        
{
    
    // Initialize tools
    tools = {
        {"select", "../assets/selection.svg", "Selection tool", "select-tool"},
        {"rectangle", "../assets/rectangle.svg", "Rectangle", "rectangle-tool"},
        {"circle", "../assets/circle.svg", "Circle", "circle-tool"},
        {"arrow", "../assets/arrow.svg", "Arrow", "arrow-tool"},
        {"line", "../assets/line.svg", "Line", "line-tool"},
        {"pen", "../assets/pen.svg", "Draw", "pen-tool"},
        {"text", "../assets/text.svg", "Text", "text-tool"},
        {"eraser", "../assets/eraser.svg", "Eraser", "eraser-tool"}
    };
    
    // Setup window
    set_title("Excalidraw-style Toolbar Demo (C++)");
    set_default_size(1000, 700);
    
    // Setup CSS
    setup_css();
    
    // Create UI
    create_toolbar();
    
    // Setup layout
    set_child(main_box);
    main_box.append(toolbar_wrapper);

    main_box.append(canvas.get_widget());
}

void UI_ToolBar::setup_css() {
    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(css_data);
    
    auto display = Gdk::Display::get_default();
    Gtk::StyleContext::add_provider_for_display(
        display, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void UI_ToolBar::create_toolbar() {
    toolbar_wrapper.add_css_class("toolbar-wrapper");
    toolbar_container.add_css_class("toolbar-container");
    
    // Add selection tool
    toolbar_container.append(*create_tool_button(tools[0]));
    toolbar_container.append(*create_separator());
    
    // Add shape tools
    for (size_t i = 1; i <= 4; ++i) {
        toolbar_container.append(*create_tool_button(tools[i]));
    }
    toolbar_container.append(*create_separator());
    
    // Add drawing tools
    toolbar_container.append(*create_tool_button(tools[5])); // pen
    toolbar_container.append(*create_tool_button(tools[6])); // text
    toolbar_container.append(*create_separator());
    
    // Add utility tools
    toolbar_container.append(*create_tool_button(tools[7])); // eraser
    
    // Center the toolbar
    toolbar_wrapper.set_center_widget(toolbar_container);
    
    // Select first tool by default
    if (!tool_buttons.empty()) {
        selected_button = tool_buttons[0];
        selected_button->add_css_class("selected");
    }
}

Gtk::Button* UI_ToolBar::create_tool_button(const ToolInfo& tool) {
    auto button = Gtk::make_managed<Gtk::Button>();
    auto icon = Gtk::make_managed<Gtk::Image>();
    try{
        auto pixbuf = Gdk::Pixbuf::create_from_file(tool.icon_name, 24, 24, true);

        icon->set(pixbuf);
        

    }catch(const Glib::Error& error){
        icon->set_from_icon_name(tool.icon_name);
    }
    
    
    button->set_child(*icon);
    button->set_tooltip_text(tool.tooltip);
    
    // Add CSS classes
    button->add_css_class("tool-button");
    button->add_css_class(tool.css_class);
    
    // Connect signal
    button->signal_clicked().connect(
        sigc::bind(sigc::mem_fun(*this, &UI_ToolBar::on_tool_selected), 
                   button, tool.name));
    
    tool_buttons.push_back(button);
    return button;
}

Gtk::Separator* UI_ToolBar::create_separator() {
    auto separator = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::VERTICAL);
    separator->add_css_class("tool-separator");
    return separator;
}


void UI_ToolBar::on_tool_selected(Gtk::Button* button, const std::string& tool_name) {
    // Remove selected class from all buttons
    for (auto* btn : tool_buttons) {
        btn->remove_css_class("selected");
    }
    
    // Add selected class to clicked button
    button->add_css_class("selected");
    selected_button = button;
   
    current_tool_name = tool_name;
    
    // Call the callback if set
    if (tool_change_callback) {
        tool_change_callback(tool_name, &canvas.get_drawing_area());
    }
}

std::string UI_ToolBar::get_current_tool(){
    return current_tool_name;
}

void UI_ToolBar::set_tool_change_callback(std::function<void(const std::string&, CairoDrawingArea*)> callback) {
    tool_change_callback = callback;
}






