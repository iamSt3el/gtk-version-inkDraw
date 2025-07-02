#include "penSettingsPanel.hpp"

PenSettingsPanel::PenSettingsPanel() {
    set_orientation(Gtk::Orientation::VERTICAL);
    set_spacing(8);
    set_margin(12);
    set_size_request(50, -1);  // Even narrower vertical panel
    
    add_css_class("pen-settings-panel");
    setup_ui();
}

void PenSettingsPanel::setup_ui() {
    // Size display - show current pen size with vertical +/- buttons
    m_size_spin = Gtk::make_managed<Gtk::SpinButton>();
    m_size_spin->set_orientation(Gtk::Orientation::VERTICAL);
    m_size_spin->set_range(0.5, 100.0);
    m_size_spin->set_value(MEDIUM_SIZE);
    m_size_spin->set_increments(0.5, 1.0);
    m_size_spin->set_digits(1);
    m_size_spin->set_size_request(40, 60);
    m_size_spin->add_css_class("size-display");
    m_size_spin->signal_value_changed().connect(
        sigc::mem_fun(*this, &PenSettingsPanel::on_size_changed));
    append(*m_size_spin);
    
    // Size preset circles - small, medium, large dots
    m_small_btn = Gtk::make_managed<Gtk::Button>();
    m_small_btn->set_size_request(32, 32);
    m_small_btn->add_css_class("size-circle");
    m_small_btn->add_css_class("size-small");
    m_small_btn->signal_clicked().connect(
        sigc::mem_fun(*this, &PenSettingsPanel::on_small_size_clicked));
    append(*m_small_btn);
    
    m_medium_btn = Gtk::make_managed<Gtk::Button>();
    m_medium_btn->set_size_request(32, 32);
    m_medium_btn->add_css_class("size-circle");
    m_medium_btn->add_css_class("size-medium");
    m_medium_btn->signal_clicked().connect(
        sigc::mem_fun(*this, &PenSettingsPanel::on_medium_size_clicked));
    append(*m_medium_btn);
    
    m_large_btn = Gtk::make_managed<Gtk::Button>();
    m_large_btn->set_size_request(32, 32);
    m_large_btn->add_css_class("size-circle");
    m_large_btn->add_css_class("size-large");
    m_large_btn->signal_clicked().connect(
        sigc::mem_fun(*this, &PenSettingsPanel::on_large_size_clicked));
    append(*m_large_btn);
    
    // Color picker button
    m_color_button = Gtk::make_managed<Gtk::ColorButton>();
    m_color_button->set_rgba(Gdk::RGBA("black"));
    m_color_button->set_size_request(36, 36);
    m_color_button->add_css_class("color-picker");
    m_color_button->signal_color_set().connect(
        sigc::mem_fun(*this, &PenSettingsPanel::on_color_changed));
    append(*m_color_button);
    
    m_opacity_scale = Gtk::make_managed<Gtk::Scale>(Gtk::Orientation::HORIZONTAL);
    m_opacity_scale->set_range(0.1, 1.0);
    m_opacity_scale->set_value(1.0);
    m_opacity_scale->set_digits(2);
    m_opacity_scale->set_visible(false); // Hidden by default
    m_opacity_scale->signal_value_changed().connect(
        sigc::mem_fun(*this, &PenSettingsPanel::on_opacity_changed));
}

double PenSettingsPanel::get_pen_size() const {
    return m_size_spin->get_value();
}

Gdk::RGBA PenSettingsPanel::get_pen_color() const {
    return m_color_button->get_rgba();
}

double PenSettingsPanel::get_pen_opacity() const {
    return m_opacity_scale->get_value();
}

void PenSettingsPanel::on_size_changed() {
    m_signal_size_changed.emit(get_pen_size());
}

void PenSettingsPanel::on_color_changed() {
    m_signal_color_changed.emit(get_pen_color());
}

void PenSettingsPanel::on_opacity_changed() {
    m_signal_opacity_changed.emit(get_pen_opacity());
}

void PenSettingsPanel::on_small_size_clicked() {
    m_size_spin->set_value(SMALL_SIZE);
    // on_size_changed() will be called automatically
}

void PenSettingsPanel::on_medium_size_clicked() {
    m_size_spin->set_value(MEDIUM_SIZE);
}

void PenSettingsPanel::on_large_size_clicked() {
    m_size_spin->set_value(LARGE_SIZE);
}

// Signal getters...
sigc::signal<void(double)> PenSettingsPanel::signal_size_changed() {
    return m_signal_size_changed;
}

sigc::signal<void(Gdk::RGBA)> PenSettingsPanel::signal_color_changed() {
    return m_signal_color_changed;
}

sigc::signal<void(double)> PenSettingsPanel::signal_opacity_changed() {
    return m_signal_opacity_changed;
}
