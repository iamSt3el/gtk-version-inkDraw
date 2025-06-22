#include "canvas.hpp"
#include "gtkmm/enums.h"

Canvas::Canvas(Gtk::Orientation orient, int spacing):box(orient, spacing){
    box.add_css_class("canvas_box");
    box.set_size_request(800, 600);

    // Creating a notebook page conatiner
    auto notebook_page = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    notebook_page->add_css_class("notebook_page");
    notebook_page->add_css_class("dotted");
    notebook_page->set_expand(true);

    box.append(*notebook_page);

    // Make drawing area expand to fill the notebook page
    drawingArea.set_expand(true);
    drawingArea.set_hexpand(true);
    drawingArea.set_vexpand(true);
    
    notebook_page->append(drawingArea);
}

Canvas::~Canvas(){

}

Gtk::Widget& Canvas::get_widget(){return box;}
