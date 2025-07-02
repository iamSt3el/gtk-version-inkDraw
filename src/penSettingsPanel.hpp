#pragma once

#include <gtkmm.h>

class PenSettingsPanel : public Gtk::Box{
    
    public:
        PenSettingsPanel();
        virtual ~PenSettingsPanel() = default;

        // Getters for pen settings
        double get_pen_size() const;
        Gdk::RGBA get_pen_color() const;
        double get_pen_opacity() const;

        // Signals for when settings change
        sigc::signal<void(double)> signal_size_changed();
        sigc::signal<void(Gdk::RGBA)> signal_color_changed();
        sigc::signal<void(double)> signal_opacity_changed();

    private:
        void setup_ui();
        void on_size_changed();
        void on_color_changed();
        void on_opacity_changed();
        void on_small_size_clicked();
        void on_medium_size_clicked();
        void on_large_size_clicked();

        // Widgets
        Gtk::SpinButton* m_size_spin;
        Gtk::Button* m_small_btn;
        Gtk::Button* m_medium_btn;
        Gtk::Button* m_large_btn;
        Gtk::ColorButton* m_color_button;
        Gtk::Scale* m_opacity_scale;

        // Preset sizes
        static constexpr double SMALL_SIZE = 2.0;
        static constexpr double MEDIUM_SIZE = 5.0;
        static constexpr double LARGE_SIZE = 10.0;

        // Signals
        sigc::signal<void(double)> m_signal_size_changed;
        sigc::signal<void(Gdk::RGBA)> m_signal_color_changed;
        sigc::signal<void(double)> m_signal_opacity_changed;

};
