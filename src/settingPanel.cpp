#include "settingPanel.hpp"
#include "gtkmm/enums.h"
#include <gtkmm.h>
#include <iostream>

SettingPanel::SettingPanel() :
    m_main_box(Gtk::Orientation::VERTICAL, 0),
    m_pattern_section(Gtk::Orientation::VERTICAL, 8),
    m_size_section(Gtk::Orientation::VERTICAL, 8),
    m_pattern_size_section(Gtk::Orientation::VERTICAL, 8),
    m_custom_size_box(Gtk::Orientation::HORIZONTAL, 8),
    m_title_label("Canvas Settings"),
    m_pattern_title("Page Pattern"),
    m_size_title("Page Size"),
    m_pattern_size_title("Pattern Scale"),
    m_width_label("Width:"),
    m_height_label("Height:")
{
    setup_content();
}

void SettingPanel::setup_content() {
    // Set popover size and styling
    set_size_request(320, 400);
    add_css_class("settings-popover");
    
    // Setup scrolled window
    m_scrolled_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    m_scrolled_window.set_child(m_main_box);
    set_child(m_scrolled_window);
    
    // Main container styling
    m_main_box.set_margin_top(20);
    m_main_box.set_margin_bottom(20);
    m_main_box.set_margin_start(20);
    m_main_box.set_margin_end(20);
    m_main_box.set_spacing(16);
    
    // Title
    m_title_label.set_markup("<span size='large' weight='bold'>Canvas Settings</span>");
    m_title_label.set_halign(Gtk::Align::START);
    m_title_label.add_css_class("settings-title");
    m_main_box.append(m_title_label);
    
    // Setup sections
    setup_pattern_section();
    create_section_separator();
    setup_size_section();
    create_section_separator();
    setup_pattern_size_section();
}

void SettingPanel::setup_pattern_section() {
    // Section title
    m_pattern_title.set_markup("<span weight='semibold'>Page Pattern</span>");
    m_pattern_title.set_halign(Gtk::Align::START);
    m_pattern_title.add_css_class("section-title");
    m_pattern_section.append(m_pattern_title);
    
    // Pattern combo
    m_pattern_combo.append("plain", "Plain");
    m_pattern_combo.append("lined", "Lined");
    m_pattern_combo.append("grid", "Grid");
    m_pattern_combo.append("dotted", "Dotted");
    m_pattern_combo.append("graph", "Graph Paper");
    m_pattern_combo.set_active_text("Plain");
    m_pattern_combo.add_css_class("settings-combo");
    m_pattern_combo.signal_changed().connect(
        sigc::mem_fun(*this, &SettingPanel::on_pattern_changed));
    
    m_pattern_section.append(m_pattern_combo);
    m_main_box.append(m_pattern_section);
}

void SettingPanel::setup_size_section() {
    // Section title
    m_size_title.set_markup("<span weight='semibold'>Page Size</span>");
    m_size_title.set_halign(Gtk::Align::START);
    m_size_title.add_css_class("section-title");
    m_size_section.append(m_size_title);
    
    // Size combo
    m_size_combo.append("a4", "A4 (210 × 297 mm)");
    m_size_combo.append("a5", "A5 (148 × 210 mm)");
    m_size_combo.append("letter", "Letter (8.5 × 11 in)");
    m_size_combo.append("legal", "Legal (8.5 × 14 in)");
    m_size_combo.append("custom", "Custom Size");
    m_size_combo.set_active_id("a4");  // Use ID instead of text
    m_size_combo.add_css_class("settings-combo");
    m_size_combo.signal_changed().connect(
        sigc::mem_fun(*this, &SettingPanel::on_page_size_changed));
    
    m_size_section.append(m_size_combo);
    
    // Custom size controls (hidden by default)
    m_width_label.add_css_class("settings-label");
    auto width_adj = Gtk::Adjustment::create(210, 50, 2000, 1, 10, 0);
    m_width_spin.set_adjustment(width_adj);
    m_width_spin.set_orientation(Gtk::Orientation::VERTICAL);
    m_width_spin.set_digits(0);
    m_width_spin.add_css_class("settings-spin");
    
    m_height_label.add_css_class("settings-label");
    auto height_adj = Gtk::Adjustment::create(297, 50, 2000, 1, 10, 0);
    m_height_spin.set_adjustment(height_adj);
    m_height_spin.set_orientation(Gtk::Orientation::VERTICAL);
    m_height_spin.set_digits(0);
    m_height_spin.add_css_class("settings-spin");
    
    // Create vertical layout for each size control
    auto width_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 4);
    width_box->append(m_width_label);
    width_box->append(m_width_spin);
    
    auto height_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 4);
    height_box->append(m_height_label);
    height_box->append(m_height_spin);
    
    m_custom_size_box.append(*width_box);
    m_custom_size_box.append(*height_box);
    m_custom_size_box.set_visible(false);
    
    m_width_spin.signal_value_changed().connect(
        sigc::mem_fun(*this, &SettingPanel::on_custom_size_changed));
    m_height_spin.signal_value_changed().connect(
        sigc::mem_fun(*this, &SettingPanel::on_custom_size_changed));
    
    m_size_section.append(m_custom_size_box);
    m_main_box.append(m_size_section);
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

void SettingPanel::on_page_size_changed() {
    std::cout << "Page size changed to: " << get_page_size() << std::endl;
    update_custom_size_visibility();
    m_signal_page_size_changed.emit(get_page_size());
}

void SettingPanel::on_pattern_size_changed() {
    double value = m_pattern_size_scale.get_value();
    m_pattern_size_value.set_text(std::to_string(value) + "x");
    m_signal_pattern_scale_changed.emit(value);
}

void SettingPanel::on_custom_size_changed() {
    // Only emit if custom size is selected
    if (get_page_size() == "custom") {
        std::cout << "Custom size changed: " << get_custom_width() << "x" << get_custom_height() << std::endl;
        m_signal_page_size_changed.emit(get_page_size());
    }
}

void SettingPanel::update_custom_size_visibility() {
    bool show_custom = (m_size_combo.get_active_id() == "custom");
    m_custom_size_box.set_visible(show_custom);
}

// Getters
std::string SettingPanel::get_page_pattern() const {
    return m_pattern_combo.get_active_id();
}

std::string SettingPanel::get_page_size() const {
    return m_size_combo.get_active_id();
}

double SettingPanel::get_pattern_size() const {
    return m_pattern_size_scale.get_value();
}

int SettingPanel::get_custom_width() const {
    return static_cast<int>(m_width_spin.get_value());
}

int SettingPanel::get_custom_height() const {
    return static_cast<int>(m_height_spin.get_value());
}

// Signal getters
sigc::signal<void(std::string)> SettingPanel::signal_pattern_changed() {
    return m_signal_pattern_changed;
}

sigc::signal<void(std::string)> SettingPanel::signal_page_size_changed() {
    return m_signal_page_size_changed;
}

sigc::signal<void(double)> SettingPanel::signal_pattern_scale_changed() {
    return m_signal_pattern_scale_changed;
}
