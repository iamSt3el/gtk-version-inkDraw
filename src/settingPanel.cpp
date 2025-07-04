#include "settingPanel.hpp"
#include "gtkmm/enums.h"
#include <gtkmm.h>
#include <iostream>

SettingPanel::SettingPanel() :
    m_main_box(Gtk::Orientation::VERTICAL, 0),
    m_pattern_section(Gtk::Orientation::VERTICAL, 8),
    m_pattern_size_section(Gtk::Orientation::VERTICAL, 8),
    m_title_label("Canvas Settings"),
    m_pattern_title("Page Pattern"),
    m_pattern_size_title("Pattern Scale")
{
    setup_content();
}

void SettingPanel::setup_content() {
    // Set popover size and styling
    set_size_request(320, 400);
    add_css_class("settings-popover");
    
    // Force white background by setting widget name
    set_name("settings-popover");
    
    // Setup scrolled window
    m_scrolled_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    m_scrolled_window.set_child(m_main_box);
    set_child(m_scrolled_window);
    
    // Main container styling
    m_main_box.set_margin_top(20);
    m_main_box.set_margin_bottom(20);
    m_main_box.set_margin_start(20);
    m_main_box.set_margin_end(0);
    m_main_box.set_spacing(16);
    
    // Title
    m_title_label.set_markup("<span size='large' weight='bold'>Canvas Settings</span>");
    m_title_label.set_halign(Gtk::Align::START);
    m_title_label.add_css_class("settings-title");
    m_main_box.append(m_title_label);
    
    // Setup sections
    setup_pattern_section();
    create_section_separator();
    create_section_separator();
    setup_pattern_size_section();
}

void SettingPanel::setup_pattern_section() {
    // Section title
    m_pattern_title.set_markup("<span weight='semibold'>Page Pattern</span>");
    m_pattern_title.set_halign(Gtk::Align::START);
    m_pattern_title.add_css_class("section-title");
    m_pattern_section.append(m_pattern_title);
    
    // Pattern dropdown using StringList
    auto pattern_list = Gtk::StringList::create({"Plain", "Lined", "Grid", "Dotted", "Graph Paper"});
    m_pattern_dropdown.set_model(pattern_list);
    m_pattern_dropdown.set_selected(0); // Select "Plain" by default
    m_pattern_dropdown.add_css_class("settings-combo");
    
    // Connect to selection change signal
    m_pattern_dropdown.property_selected().signal_changed().connect(
        sigc::mem_fun(*this, &SettingPanel::on_pattern_changed));
    
    m_pattern_section.append(m_pattern_dropdown);
    m_main_box.append(m_pattern_section);
}

void SettingPanel::setup_pattern_size_section() {
    // Section title
    m_pattern_size_title.set_markup("<span weight='semibold'>Pattern Scale</span>");
    m_pattern_size_title.set_halign(Gtk::Align::START);
    m_pattern_size_title.add_css_class("section-title");
    m_pattern_size_section.append(m_pattern_size_title);
    
    // Scale slider
    auto scale_adj = Gtk::Adjustment::create(1.0, 0.5, 3.0, 0.1, 0.5, 0);
    m_pattern_size_scale.set_adjustment(scale_adj);
    m_pattern_size_scale.set_digits(1);
    m_pattern_size_scale.set_hexpand(true);
    m_pattern_size_scale.add_css_class("settings-scale");
    m_pattern_size_scale.signal_value_changed().connect(
        sigc::mem_fun(*this, &SettingPanel::on_pattern_size_changed));
    
    // Value label
    m_pattern_size_value.set_text("1.0x");
    m_pattern_size_value.add_css_class("settings-value");
    
    m_pattern_size_section.append(m_pattern_size_scale);
    m_pattern_size_section.append(m_pattern_size_value);
    m_main_box.append(m_pattern_size_section);
}

void SettingPanel::create_section_separator() {
    auto separator = Gtk::make_managed<Gtk::Separator>();
    separator->add_css_class("settings-separator");
    m_main_box.append(*separator);
}

// Event handlers
void SettingPanel::on_pattern_changed() {
    m_signal_pattern_changed.emit(get_page_pattern());
}


void SettingPanel::on_pattern_size_changed() {
    double value = m_pattern_size_scale.get_value();
    m_pattern_size_value.set_text(std::to_string(value) + "x");
    m_signal_pattern_scale_changed.emit(value);
}


// Getters
std::string SettingPanel::get_page_pattern() const {
    auto selected = m_pattern_dropdown.get_selected();
    switch(selected) {
        case 0: return "plain";
        case 1: return "lined";
        case 2: return "grid";
        case 3: return "dotted";
        case 4: return "graph";
        default: return "plain";
    }
}

double SettingPanel::get_pattern_size() const {
    return m_pattern_size_scale.get_value();
}


// Signal getters
sigc::signal<void(std::string)> SettingPanel::signal_pattern_changed() {
    return m_signal_pattern_changed;
}


sigc::signal<void(double)> SettingPanel::signal_pattern_scale_changed() {
    return m_signal_pattern_scale_changed;
}

