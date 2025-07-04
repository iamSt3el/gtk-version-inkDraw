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
    background-image: linear-gradient(to bottom, transparent calc(24px * var(--grid-scale, 1)), #ddd calc(24px * var(--grid-scale, 1)), #ddd calc(25px * var(--grid-scale, 1)), transparent calc(25px * var(--grid-scale, 1)));
    background-size: 100% calc(25px * var(--grid-scale, 1));
    background-repeat: repeat;
}

.notebook_page.grid{
    background-image: 
        linear-gradient(to right, #ddd 1px, transparent 1px),
        linear-gradient(to bottom, #ddd 1px, transparent 1px);
    background-size: calc(20px * var(--grid-scale, 1)) calc(20px * var(--grid-scale, 1));
}

.notebook_page.dotted{
    background-image: radial-gradient(circle, #ddd 1px, transparent 1px);
    background-size: calc(15px * var(--grid-scale, 1)) calc(15px * var(--grid-scale, 1));
}

.notebook_page.graph{
    background-image: 
        linear-gradient(to right, #eee 1px, transparent 1px),
        linear-gradient(to bottom, #eee 1px, transparent 1px),
        linear-gradient(to right, #ddd 1px, transparent 1px),
        linear-gradient(to bottom, #ddd 1px, transparent 1px);
    background-size: 
        calc(10px * var(--grid-scale, 1)) calc(10px * var(--grid-scale, 1)), 
        calc(10px * var(--grid-scale, 1)) calc(10px * var(--grid-scale, 1)), 
        calc(50px * var(--grid-scale, 1)) calc(50px * var(--grid-scale, 1)), 
        calc(50px * var(--grid-scale, 1)) calc(50px * var(--grid-scale, 1));
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

/* Pen settings vertical overlay panel */
.pen-settings-panel {
    background: rgba(255, 255, 255, 0.95);
    border-radius: 12px;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.1);
    padding: 6px;
    min-width: 50px;
    border: 1px solid rgba(0, 0, 0, 0.06);
}

/* Panel icons styling */
.panel-icon {
    color: #374151;
    padding: 8px;
    margin: 2px 0;
}

.plus-icon {
    color: #374151;
    font-size: 20px;
    font-weight: bold;
    padding: 8px;
    margin: 2px 0;
    text-align: center;
}

/* Size display */
.size-display {
    background: rgba(255, 255, 255, 0.95);
    color: #374151;
    border: 1px solid rgba(0, 0, 0, 0.1);
    border-radius: 4px;
    margin: 2px 0;
}

.size-display entry,
.size-display entry text {
    background: rgba(255, 255, 255, 1.0) !important;
    color: #374151 !important;
    text-align: center;
    border: none !important;
    border-radius: 3px;
    box-shadow: none !important;
    outline: none !important;
}

.size-display entry:focus,
.size-display entry:focus text {
    background: rgba(255, 255, 255, 1.0) !important;
    color: #374151 !important;
    border: none !important;
    box-shadow: 0 0 0 1px rgba(99, 102, 241, 0.4) !important;
    outline: none !important;
}

.size-display button {
    background: transparent;
    border: 1px solid rgba(0, 0, 0, 0.1);
    border-radius: 5px;
    color: #374151;
}

.size-display button:hover {
    background: rgba(99, 102, 241, 0.1);
    border-color: rgba(99, 102, 241, 0.3);
}

/* Size circle buttons */
.size-circle {
    background: transparent;
    border: 2px solid rgba(0, 0, 0, 0.2);
    border-radius: 40px;
    margin: 3px 0;
    min-width: 32px;
    min-height: 32px;
}

.size-circle:hover {
    border-color: rgba(99, 102, 241, 0.6);
    background: rgba(99, 102, 241, 0.1);
}

.size_circle.selected{
border: 2px solid rgba(1, 1, 1, 0.2);
}

.size-small {
    background: radial-gradient(circle, #374151 2px, transparent 2px);
}

.size-medium {
    background: radial-gradient(circle, #374151 4px, transparent 4px);
}

.size-large {
    background: radial-gradient(circle, #374151 6px, transparent 6px);
}

/* Color picker button */
.color-picker {
    border-radius: 6px;
    border: 2px solid rgba(0, 0, 0, 0.1);
    margin: 3px 0;
    box-shadow: none;
    background: rgba(255, 255, 255, 0.9);
}

.color-picker:hover {
    border-color: rgba(99, 102, 241, 0.4);
    background: rgba(255, 255, 255, 1.0);
}

.color-picker button {
    border: none;
    box-shadow: none;
    border-radius: 4px;
    background: transparent;
}

.color-picker button:hover {
    background: transparent;
    box-shadow: none;
    border: none;
}

/* Settings Popover - Force white background */
popover {
    background: rgba(255, 255, 255, 0.98) !important;
    border-radius: 12px;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.12);
    border: 1px solid rgba(0, 0, 0, 0.06);
}

/* Target specific popover by name */
popover#settings-popover {
    background: rgba(255, 255, 255, 1.0) !important;
}

popover contents {
    background: rgba(255, 255, 255, 0.98) !important;
    border-radius: 12px;
}

popover scrolledwindow {
    background: rgba(255, 255, 255, 0.98) !important;
}

popover scrolledwindow > viewport {
    background: rgba(255, 255, 255, 0.98) !important;
}

.settings-popover {
    background: rgba(255, 255, 255, 0.98) !important;
}

/* Alternative targeting */
window popover {
    background: rgba(255, 255, 255, 0.98) !important;
}

window popover contents {
    background: rgba(255, 255, 255, 0.98) !important;
}

.settings-title {
    color: #111827;
    margin-bottom: 8px;
}

.section-title {
    color: #374151;
    margin-bottom: 6px;
    font-size: 0.95em;
}

.settings-separator {
    background: rgba(0, 0, 0, 0.08);
    min-height: 1px;
    margin: 4px 0;
}

.settings-combo {
    background: rgba(248, 249, 250, 0.9);
    border: 1px solid rgba(0, 0, 0, 0.1);
    border-radius: 6px;
    padding: 8px 12px;
    color: #374151;
}

.settings-combo:hover {
    border-color: rgba(99, 102, 241, 0.3);
    background: rgba(248, 249, 250, 1.0);
}

.settings-combo:focus {
    border-color: rgba(99, 102, 241, 0.5);
    box-shadow: 0 0 0 2px rgba(99, 102, 241, 0.1);
}

.settings-label {
    color: #374151;
    font-size: 0.9em;
    min-width: 60px;
}

.settings-spin {
    background: rgba(248, 249, 250, 0.9);
    border: 1px solid rgba(0, 0, 0, 0.1);
    border-radius: 4px;
    color: #374151;
    min-width: 80px;
}

.settings-spin entry,
.settings-spin entry text {
    background: rgba(255, 255, 255, 1.0) !important;
    color: #374151 !important;
    border: none !important;
    box-shadow: none !important;
    outline: none !important;
}

.settings-spin button {
    background: transparent;
    border: 1px solid rgba(0, 0, 0, 0.1);
    border-radius: 3px;
    color: #374151;
}

.settings-spin button:hover {
    background: rgba(99, 102, 241, 0.1);
    border-color: rgba(99, 102, 241, 0.3);
}

.settings-scale {
    background: transparent;
}

.settings-scale trough {
    background: rgba(0, 0, 0, 0.1);
    border-radius: 4px;
    min-height: 6px;
}

.settings-scale slider {
    background: #6366f1;
    border: 2px solid white;
    border-radius: 50%;
    min-width: 18px;
    min-height: 18px;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
}

.settings-scale highlight {
    background: #6366f1;
    border-radius: 4px;
}

.settings-value {
    color: #6b7280;
    font-size: 0.85em;
    text-align: center;
    margin-top: 4px;
}
)";

// Constructor
UI_ToolBar::UI_ToolBar()
    : main_box(Gtk::Orientation::VERTICAL, 0),
      toolbar_container(Gtk::Orientation::HORIZONTAL, 2),
      canvas(Gtk::Orientation::VERTICAL, 0),
      selected_button(nullptr),
      current_tool_name("select"),
      settingPanel(nullptr),
      m_pen_settings_panel(nullptr)
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
        {"eraser", "../assets/eraser.svg", "Eraser", "eraser-tool"},
        {"setting", "../assets/setting.svg", "Setting", "setting"}
    };
    base_css_provider = Gtk::CssProvider::create();
    scale_css_provider = Gtk::CssProvider::create();
    // Setup window
    set_title("Excalidraw-style Toolbar Demo (C++)");
    set_default_size(1000, 700);
    
    // Setup CSS
    setup_css();
    
    // Create UI
    create_toolbar();
    
    // Setup pen settings panel as overlay
    m_pen_settings_panel = Gtk::make_managed<PenSettingsPanel>();
    
    // Setup layout with overlay
    set_child(main_box);
    main_box.append(toolbar_wrapper);
    main_box.append(canvas_overlay);
    
    // Add canvas as base layer and pen settings as overlay
    canvas_overlay.set_child(canvas.get_widget());
    canvas_overlay.add_overlay(*m_pen_settings_panel);
    
    // Position the pen settings panel on the left side, vertically centered
    m_pen_settings_panel->set_halign(Gtk::Align::START);
    m_pen_settings_panel->set_valign(Gtk::Align::START);
    m_pen_settings_panel->set_margin_start(20);
    m_pen_settings_panel->set_margin_top(20);
    
    // Hide by default since select is default tool  
    m_pen_settings_panel->set_visible(false);

    settingButton->signal_clicked().connect(
        sigc::mem_fun(*this, &UI_ToolBar::on_setting_clicked)
    );
    
    // Setup pen settings connections
    setup_pen_settings_connections();
}

void UI_ToolBar::setup_pen_settings_connections() {
    // Connect pen settings signals to drawing area
    if (m_pen_settings_panel) {
        m_pen_settings_panel->signal_size_changed().connect([this](double size) {
            std::cout << "Size changed to: " << size << std::endl; // Debug
            canvas.get_drawing_area().set_stroke_width(size);
        });
        
        m_pen_settings_panel->signal_color_changed().connect([this](Gdk::RGBA color) {
            std::cout << "Color changed" << std::endl; // Debug
            Color drawing_color(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
            canvas.get_drawing_area().set_stroke_color(drawing_color);
        });
        
        m_pen_settings_panel->signal_opacity_changed().connect([this](double opacity) {
            std::cout << "Opacity changed to: " << opacity << std::endl; // Debug
            canvas.get_drawing_area().set_stroke_opacity(opacity);
        });
    }
}

void UI_ToolBar::setup_css() {
    //auto css_provider = Gtk::CssProvider::create();
    base_css_provider->load_from_data(css_data);
    
    auto display = Gdk::Display::get_default();
    Gtk::StyleContext::add_provider_for_display(
        display, base_css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    Gtk::StyleContext::add_provider_for_display(
            display, scale_css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION            );
}

void UI_ToolBar::set_setting_panel(){
    settingPanel = Gtk::make_managed<SettingPanel>();
    if(settingPanel){
        settingPanel->set_parent(*settingButton);
        settingPanel->set_autohide(true);  // Auto-hide when clicking outside
        settingPanel->set_has_arrow(true);  // Add arrow pointing to button
        settingPanel->set_cascade_popdown(true);  // Fix for GTK4 bug #4529
        
        // Connect settings panel signals to canvas
        settingPanel->signal_pattern_changed().connect([this](std::string pattern) {
            std::cout << "Applying pattern: " << pattern << std::endl;
            canvas.set_page_pattern(pattern);
        });
        
        
        settingPanel->signal_pattern_scale_changed().connect([this](double scale) {
            std::cout << "Pattern scale changed to: " << scale << std::endl;
            // This would update CSS variables for pattern scaling
            // Implementation depends on how you want to handle pattern scaling
            char css[64];

            snprintf(css, sizeof(css), ".notebook_page { --grid-scale: %.2f;}", scale);
            scale_css_provider->load_from_data(css);
            });
    }
}

void UI_ToolBar::on_setting_clicked(){
    if(settingPanel){
        if(settingPanel->get_visible()) {
            settingPanel->popdown();  // Close if already open
        } else {
            settingPanel->popup();    // Open if closed
        }
    }
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
    
    // Add utility tools
    toolbar_container.append(*create_tool_button(tools[7])); // eraser
   

    toolbar_container.append(*create_separator());
    settingButton = create_tool_button(tools[8]);
    toolbar_container.append(*settingButton);
    
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
    
    // Show/hide pen settings panel based on selected tool
    if (m_pen_settings_panel) {
        if (tool_name == "pen") {
            m_pen_settings_panel->set_visible(true);
        } else {
            m_pen_settings_panel->set_visible(false);
        }
    }
    
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






