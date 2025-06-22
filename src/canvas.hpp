#pragma once

#include "gtkmm/enums.h"
#include <gtkmm.h>
#include "drawingLogic.hpp"

class Canvas{
    private:
        Gtk::Box box;
        
    public:
        Canvas(Gtk::Orientation orient, int spacing);
        ~Canvas();

        Gtk::Widget& get_widget();
        CairoDrawingArea& get_drawing_area() { return drawingArea; }
        CairoDrawingArea drawingArea;
};
