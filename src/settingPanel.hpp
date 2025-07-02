#pragma once
#include <gtkmm.h>

class SettingPanel : public Gtk::Popover {
    public:
        SettingPanel();
        void setup_content();

        // Getters for settings
        std::string get_page_pattern() const;
        std::string get_page_size() const;
        double get_pattern_size() const;
        int get_custom_width() const;
        int get_custom_height() const;
        
        // Signals for settings changes
        sigc::signal<void(std::string)> signal_pattern_changed();
        sigc::signal<void(std::string)> signal_page_size_changed();
        sigc::signal<void(double)> signal_pattern_scale_changed();

    protected:
        void on_pattern_changed();
        void on_page_size_changed();
        void on_pattern_size_changed();
        void on_custom_size_changed();

        // Main container
        Gtk::Box m_main_box;
        Gtk::ScrolledWindow m_scrolled_window;
        
        // Title
        Gtk::Label m_title_label;
        
        // Page Pattern Section
        Gtk::Box m_pattern_section;
        Gtk::Label m_pattern_title;
        Gtk::ComboBoxText m_pattern_combo;
        
        // Page Size Section  
        Gtk::Box m_size_section;
        Gtk::Label m_size_title;
        Gtk::ComboBoxText m_size_combo;
        Gtk::Box m_custom_size_box;
        Gtk::Label m_width_label;
        Gtk::SpinButton m_width_spin;
        Gtk::Label m_height_label;
        Gtk::SpinButton m_height_spin;
        
        // Pattern Size Section
        Gtk::Box m_pattern_size_section;
        Gtk::Label m_pattern_size_title;
        Gtk::Scale m_pattern_size_scale;
        Gtk::Label m_pattern_size_value;

    private:
        void create_section_separator();
        void setup_pattern_section();
        void setup_size_section();
        void setup_pattern_size_section();
        void update_custom_size_visibility();
        
        // Signal objects
        sigc::signal<void(std::string)> m_signal_pattern_changed;
        sigc::signal<void(std::string)> m_signal_page_size_changed;
        sigc::signal<void(double)> m_signal_pattern_scale_changed;
};
