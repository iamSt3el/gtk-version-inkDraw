#pragma once

#include "gtkmm/enums.h"
#include <gtkmm.h>
#include "drawingLogic.hpp"

class Canvas{
    private:
        Gtk::Box box;
        Gtk::Box* notebook_page;
        std::string current_pattern = "plain";
        
    public:
        Canvas(Gtk::Orientation orient, int spacing);
        ~Canvas();

        Gtk::Widget& get_widget();
        CairoDrawingArea& get_drawing_area() { return drawingArea; }
        
        // Page settings methods
        void set_page_pattern(const std::string& pattern);
        void set_page_size(int width, int height);
        std::string get_page_pattern() const { return current_pattern; }
        
        CairoDrawingArea drawingArea;
};
