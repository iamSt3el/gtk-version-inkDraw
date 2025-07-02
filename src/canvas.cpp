#include "canvas.hpp"
#include "gtkmm/enums.h"
#include <iostream>

Canvas::Canvas(Gtk::Orientation orient, int spacing):box(orient, spacing){
    box.add_css_class("canvas_box");
    box.set_size_request(800, 600);

    // Creating a notebook page container
    notebook_page = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    notebook_page->add_css_class("notebook_page");
    // Only add pattern class if not plain
    if (current_pattern != "plain") {
        notebook_page->add_css_class(current_pattern);
    }
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

void Canvas::set_page_pattern(const std::string& pattern) {
    // Remove current pattern class
    notebook_page->remove_css_class(current_pattern);
    
    // Update current pattern
    current_pattern = pattern;
    
    // Add new pattern class (only if not "plain")
    if (pattern != "plain") {
        notebook_page->add_css_class(current_pattern);
    }
}

void Canvas::set_page_size(int width, int height) {
    std::cout << "Setting canvas size to: " << width << "x" << height << std::endl;
    box.set_size_request(width, height);
    notebook_page->set_size_request(width, height);
    // Force a redraw
    box.queue_resize();
}
