#pragma once

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <vector>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>

struct Point {
    double x, y;
    long long timestamp;
    
    Point(double x, double y);
};

struct Color {
    double r, g, b;
    Color(double r = 0.0, double g = 0.0, double b = 0.0) : r(r), g(g), b(b) {}
};

struct Rect{
    double x, y;
    double width, height;
    Color color;
    
    Rect(double x, double y, double width, double height, Color color);
};

struct Circle_Data{
    double x, y;
    double r;
    Color color;

    Circle_Data(double x, double y, double r, Color color);
};

// Bounding box for selection and collision detection
struct BoundingBox {
    double x, y, width, height;
    
    BoundingBox(double x = 0, double y = 0, double w = 0, double h = 0) 
        : x(x), y(y), width(w), height(h) {}
    
    bool contains_point(double px, double py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

// Selection handle positions for resizing
enum class HandlePosition {
    TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT,
    TOP, BOTTOM, LEFT, RIGHT, NONE
};

// Base class for all drawable objects
class DrawableObject {
public:
    virtual ~DrawableObject() = default;
    
    // Core functionality
    virtual void draw(const Cairo::RefPtr<Cairo::Context>& cr) const = 0;
    virtual BoundingBox get_bounding_box() const = 0;
    virtual bool hit_test(double x, double y) const = 0;
    virtual std::string get_type() const = 0;
    
    // Transformation methods
    virtual void translate(double dx, double dy) = 0;
    virtual void scale(double scale_x, double scale_y, double origin_x, double origin_y) = 0;
    
    // Selection state
    bool is_selected = false;
    
    // Handle detection for resizing
    HandlePosition get_handle_at_point(double x, double y) const;
    void draw_selection_handles(const Cairo::RefPtr<Cairo::Context>& cr) const;
};

class Stroke {
public:
    std::vector<Point> points;
    Color color;
    double width;
    
    Stroke(double w = 3.0, Color col = Color(0.0, 0.0, 0.8)) 
        : width(w), color(col) {}
    
    void add_point(double x, double y);
};

// Concrete drawable object implementations
class StrokeObject : public DrawableObject {
private:
    Stroke stroke;
    
public:
    StrokeObject(const Stroke& s) : stroke(s) {}
    
    void draw(const Cairo::RefPtr<Cairo::Context>& cr) const override;
    BoundingBox get_bounding_box() const override;
    bool hit_test(double x, double y) const override;
    std::string get_type() const override { return "stroke"; }
    
    void translate(double dx, double dy) override;
    void scale(double scale_x, double scale_y, double origin_x, double origin_y) override;
    
    const Stroke& get_stroke() const { return stroke; }
    Stroke& get_stroke() { return stroke; }
};

class RectangleObject : public DrawableObject {
private:
    Rect rect;
    
public:
    RectangleObject(const Rect& r) : rect(r) {}
    
    void draw(const Cairo::RefPtr<Cairo::Context>& cr) const override;
    BoundingBox get_bounding_box() const override;
    bool hit_test(double x, double y) const override;
    std::string get_type() const override { return "rectangle"; }
    
    void translate(double dx, double dy) override;
    void scale(double scale_x, double scale_y, double origin_x, double origin_y) override;
    
    const Rect& get_rect() const { return rect; }
    Rect& get_rect() { return rect; }
};

class CircleObject : public DrawableObject {
private:
    Circle_Data circle;
    
public:
    CircleObject(const Circle_Data& c) : circle(c) {}
    
    void draw(const Cairo::RefPtr<Cairo::Context>& cr) const override;
    BoundingBox get_bounding_box() const override;
    bool hit_test(double x, double y) const override;
    std::string get_type() const override { return "circle"; }
    
    void translate(double dx, double dy) override;
    void scale(double scale_x, double scale_y, double origin_x, double origin_y) override;
    
    const Circle_Data& get_circle() const { return circle; }
    Circle_Data& get_circle() { return circle; }
};

// Selection manager for handling multiple selections
class SelectionManager {
private:
    std::vector<std::shared_ptr<DrawableObject>> selected_objects;
    
public:
    void clear_selection();
    void add_to_selection(std::shared_ptr<DrawableObject> obj);
    void remove_from_selection(std::shared_ptr<DrawableObject> obj);
    bool is_selected(std::shared_ptr<DrawableObject> obj) const;
    
    const std::vector<std::shared_ptr<DrawableObject>>& get_selected() const { return selected_objects; }
    bool has_selection() const { return !selected_objects.empty(); }
    
    BoundingBox get_selection_bounds() const;
    void move_selection(double dx, double dy);
    void scale_selection(double scale_x, double scale_y, double origin_x, double origin_y);
};

class Circle{
    public:
        std::vector<Circle_Data> circles;

        void add_circle(double x, double y, double r, Color color);
};

class Rectangle{
    public:
        std::vector<Rect> rects;

        void add_rect(double x, double y, double width, double height, Color color);
};



class Eraser{
    private:
        double eraser_radius;
    public:
        std::vector<Stroke> stroke_to_erase;
        std::vector<Rect> rectangle_to_erase;
        std::vector<Circle_Data> circle_to_erase;

        void eraser_while_erasing();
        void eraser_ended_erasing();
};

class CairoDrawingArea : public Gtk::DrawingArea {
private:
    //------ UNIFIED OBJECT SYSTEM ------
    std::vector<std::shared_ptr<DrawableObject>> drawable_objects;
    SelectionManager selection_manager;
    
    //------ VARIABLES FOR TOOLBAR TOOLS ------
    // Legacy variables (will be phased out)
    std::vector<Stroke> completed_strokes;
    Stroke current_stroke;

    // Rectangle Related variables
    std::vector<Rectangle> completed_rectangles;
    Rectangle current_rectangle;
    
    // Circle Related variables
    std::vector<Circle> completed_circles;
    Circle current_circle;

    // Eraser Related variable
    Eraser current_eraser;

    // Tool states
    bool is_drawing;
    bool is_drawing_rectangle;
    bool is_drawing_circle;
    bool is_erasing;
    bool is_selecting;
    bool is_moving;
    bool is_moving_selection;
    bool is_resizing;
    std::string current_tool;
    
    // Selection state for legacy objects
    std::vector<int> selected_stroke_indices;
    std::vector<int> selected_rectangle_indices;
    std::vector<int> selected_circle_indices;
    
    // Selection tool variables
    Point selection_start;
    HandlePosition current_handle;

    // Rectangle points
    Point rectangle_start;


    Point current_mouse_pos;

    // Circle points
    Point circle_start;

    Color default_rectangle_color = Color(0.0, 0.0, 0.0);
    Color default_circle_color = Color(0.0, 0.0, 0.0);
public:
    CairoDrawingArea();
    ~CairoDrawingArea();
    
    // Public interface
    void clear_canvas();
    void undo();
    void set_stroke_width(double width);
    void set_stroke_color(const Color& color);
    void set_rectangle_color(const Color& color);
    void set_drawing_state(std::string state);
    void set_current_cursor();
    void clear_selection();
    
protected:
    // GTK callbacks
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    
private:
    // Input handling
    void setup_input_handling();
    
    // ------ DRAWING FUNCTIONS -----
    //
    // Strokes
    void draw_stroke(const Cairo::RefPtr<Cairo::Context>& cr, const Stroke& stroke);
    void draw_smooth_stroke(const Cairo::RefPtr<Cairo::Context>& cr, const Stroke& stroke);
    
    // Rectangle
    void draw_rectangle(const Cairo::RefPtr<Cairo::Context>& cr, const Rectangle& rectangle);
    void draw_rectangle_preview(const Cairo::RefPtr<Cairo::Context>& cr, double start_x, double start_y, double end_x, double end_y);

    // Circle
    void draw_circle(const Cairo::RefPtr<Cairo::Context>& cr, const Circle& circle);
    void draw_circle_preview(const Cairo::RefPtr<Cairo::Context>& cr, double start_x, double start_y, double r);

    // Eraser
    void draw_erasing_preview(const Cairo::RefPtr<Cairo::Context>& cr, const Stroke& stroke);
    void draw_rectangle_erasing_preview(const Cairo::RefPtr<Cairo::Context>& cr, const Rect& rect);

    
    // Eraser collision detection
    bool is_stroke_in_eraser_radius(const Stroke& stroke, double eraser_x, double eraser_y, double radius);
    bool is_rect_in_eraser_radius(const Rect& rect, double eraser_x, double eraser_y, double radius);
    bool is_circle_in_eraser_radius(const Circle_Data& circle, double eraser_x, double eraser_y, double radius);
    void update_eraser_collision(double x, double y);
    void update_eraser_preview(double x, double y);
    
    // Selection system methods
    std::shared_ptr<DrawableObject> find_object_at_point(double x, double y);
    HandlePosition find_handle_at_point(double x, double y);
    void update_selection(double x, double y, bool multi_select = false);
    void start_move_operation(double x, double y);
    void start_resize_operation(double x, double y, HandlePosition handle);
    void perform_move(double x, double y);
    void perform_resize(double x, double y);
    
    // Object management
    void add_drawable_object(std::shared_ptr<DrawableObject> obj);
    void remove_drawable_object(std::shared_ptr<DrawableObject> obj);
    
    // Selection functions for legacy objects
    void clear_all_selections();
    void select_objects_in_rectangle(double x1, double y1, double x2, double y2);
    void move_selected_objects(double dx, double dy);
    void draw_selection_rectangle(const Cairo::RefPtr<Cairo::Context>& cr, double x1, double y1, double x2, double y2);
    void draw_selection_highlights(const Cairo::RefPtr<Cairo::Context>& cr);
    bool is_point_in_stroke(const Stroke& stroke, double x, double y, double tolerance = 5.0);
    bool is_point_in_rectangle(const Rect& rect, double x, double y);
    bool is_point_in_circle(const Circle_Data& circle, double x, double y);
    
    // Smoothing helpers
    std::vector<Point> simplify_stroke(const std::vector<Point>& points, double tolerance = 2.0);
    std::vector<Point> ultra_smooth_stroke(const std::vector<Point>& points);
    std::vector<Point> interpolate_catmull_rom(const std::vector<Point>& points, int segments_per_curve = 8);
    Point catmull_rom_point(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t);
    double point_distance(const Point& p1, const Point& p2);
};

// Tool change handler function
void on_tool_changed(const std::string& tool_name, CairoDrawingArea* drawing_area);

