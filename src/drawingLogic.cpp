#include "drawingLogic.hpp"
#include <iostream>
#include <cmath>

// Point implementation
Point::Point(double x, double y) : x(x), y(y) {
    timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Rect implementation
Rect::Rect(double x, double y, double width, double height, Color color) : x(x), y(y), width(width), height(height), color(color) {}

// Circle implementaion
Circle_Data::Circle_Data(double x, double y, double r, Color color) : x(x), y(y), r(r), color(color){}

// Stroke implementation
void Stroke::add_point(double x, double y) {
    raw_points.emplace_back(x, y);
    
    // Calculate smooth points in real-time for better UX
    if (raw_points.size() < 2) {
        points = raw_points;
        return;
    }
    
    // Step 1: Light simplification to remove micro-jitter
    std::vector<Point> simplified;
    if (raw_points.size() <= 2) {
        simplified = raw_points;
    } else {
        simplified.push_back(raw_points[0]); // Always keep first point
        
        for (size_t i = 1; i < raw_points.size() - 1; i++) {
            const Point& prev = simplified.back();
            const Point& current = raw_points[i];
            
            // Calculate distance from current point to previous point
            double dx = current.x - prev.x;
            double dy = current.y - prev.y;
            double dist = sqrt(dx * dx + dy * dy);
            
            // Keep point if it's far enough from the previous point
            if (dist >= 0.5) {
                simplified.push_back(current);
            }
        }
        
        simplified.push_back(raw_points.back()); // Always keep last point
    }
    
    // Step 2: Catmull-Rom interpolation for ultra-smooth curves
    if (simplified.size() < 2) {
        points = simplified;
    } else if (simplified.size() == 2) {
        points = simplified;
    } else {
        points.clear();
        points.reserve(simplified.size() * 12);
        
        // Add first point
        points.push_back(simplified[0]);
        
        // Interpolate between each consecutive pair of points
        for (size_t i = 0; i < simplified.size() - 1; i++) {
            // Get the four control points for Catmull-Rom spline
            Point p0 = (i > 0) ? simplified[i-1] : simplified[i];
            Point p1 = simplified[i];
            Point p2 = simplified[i+1];
            Point p3 = (i+2 < simplified.size()) ? simplified[i+2] : simplified[i+1];
            
            // Generate many interpolated points for super smooth curves
            for (int j = 1; j <= 12; j++) {
                double t = (double)j / 12;
                double t2 = t * t;
                double t3 = t2 * t;
                
                // Catmull-Rom spline formula
                double x = 0.5 * ((2.0 * p1.x) +
                                 (-p0.x + p2.x) * t +
                                 (2.0 * p0.x - 5.0 * p1.x + 4.0 * p2.x - p3.x) * t2 +
                                 (-p0.x + 3.0 * p1.x - 3.0 * p2.x + p3.x) * t3);
                
                double y = 0.5 * ((2.0 * p1.y) +
                                 (-p0.y + p2.y) * t +
                                 (2.0 * p0.y - 5.0 * p1.y + 4.0 * p2.y - p3.y) * t2 +
                                 (-p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y) * t3);
                
                points.emplace_back(x, y);
            }
        }
    }
}

// Rectangle implementation
void Rectangle::add_rect(double x, double y, double width, double height, Color color) {
    rects.emplace_back(x, y, width, height, color);
}

// Circle implementation
void Circle::add_circle(double x, double y, double r, Color color){
    circles.emplace_back(x, y, r, color);
}

// Cairo Drawing Area implementation
CairoDrawingArea::CairoDrawingArea() : is_drawing(false), is_drawing_rectangle(false), is_drawing_circle(false), is_erasing(false), is_selecting(false), is_moving(false), is_moving_selection(false), is_resizing(false), rectangle_start(0, 0), current_mouse_pos(0, 0), circle_start(0, 0), selection_start(0, 0), current_handle(HandlePosition::NONE), last_redraw_time(std::chrono::steady_clock::now())
{
    set_size_request(800, 600);
    
    // Initialize background surface for caching completed strokes (like SVG layer)
    initialize_background_surface(800, 600);
    
    // Set up drawing function
    set_draw_func(sigc::mem_fun(*this, &CairoDrawingArea::on_draw));

    auto cursor = Gdk::Cursor::create("default");
    set_cursor(cursor);
    
    setup_input_handling();
}

// Setting panel function


CairoDrawingArea::~CairoDrawingArea() {
}

void CairoDrawingArea::setup_input_handling() {
    auto motion_controller = Gtk::EventControllerMotion::create();
    auto click_gesture = Gtk::GestureClick::create();
    
    // Mouse press - start drawing
    click_gesture->signal_pressed().connect([this](int n_press, double x, double y){
        if(current_tool == "pen"){ 
            is_drawing = true;
            current_stroke = Stroke(current_pen_width, current_pen_color);
            current_stroke.add_point(x, y);
        }
        else if(current_tool == "rectangle") {
            is_drawing_rectangle = true;
            rectangle_start = Point(x, y);
            current_rectangle = Rectangle();
        }
        else if(current_tool == "circle"){
             is_drawing_circle = true;
             circle_start = Point(x, y);
             current_circle = Circle();
        }
        else if(current_tool == "eraser") {
            is_erasing = true;
            // Don't clear the vectors here - they'll be cleared on mouse release
        }
        else if(current_tool == "select") {
            // Check if clicking on an already selected object to start moving
            bool clicked_on_selected = false;
            
            // Check strokes
            for (int i = 0; i < completed_strokes.size(); i++) {
                if (std::find(selected_stroke_indices.begin(), selected_stroke_indices.end(), i) != selected_stroke_indices.end()) {
                    if (is_point_in_stroke(completed_strokes[i], x, y)) {
                        clicked_on_selected = true;
                        break;
                    }
                }
            }
            
            if (!clicked_on_selected) {
                // Check rectangles
                for (int i = 0; i < completed_rectangles.size(); i++) {
                    if (std::find(selected_rectangle_indices.begin(), selected_rectangle_indices.end(), i) != selected_rectangle_indices.end()) {
                        for (const auto& rect : completed_rectangles[i].rects) {
                            if (is_point_in_rectangle(rect, x, y)) {
                                clicked_on_selected = true;
                                break;
                            }
                        }
                        if (clicked_on_selected) break;
                    }
                }
            }
            
            if (!clicked_on_selected) {
                // Check circles
                for (int i = 0; i < completed_circles.size(); i++) {
                    if (std::find(selected_circle_indices.begin(), selected_circle_indices.end(), i) != selected_circle_indices.end()) {
                        for (const auto& circle : completed_circles[i].circles) {
                            if (is_point_in_circle(circle, x, y)) {
                                clicked_on_selected = true;
                                break;
                            }
                        }
                        if (clicked_on_selected) break;
                    }
                }
            }
            
            if (clicked_on_selected) {
                // Start moving selected objects
                is_moving_selection = true;
                selection_start = Point(x, y);
            } else {
                // Start selection rectangle or clear selection
                is_selecting = true;
                selection_start = Point(x, y);
                clear_all_selections();
            }
        }
    });
    
    // Mouse motion - add points while drawing
    motion_controller->signal_motion().connect([this](double x, double y){
        if (is_drawing) {
            // Reduce threshold for much denser point collection
            if (current_stroke.points.empty() || 
                point_distance(current_stroke.points.back(), Point(x, y)) >= 0.5) {
                current_stroke.add_point(x, y);
                
                // Frame rate limiting: only redraw at 60fps max (16.67ms intervals)
                auto now = std::chrono::steady_clock::now();
                auto time_since_last_redraw = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_redraw_time);
                if (time_since_last_redraw.count() >= 16) { // 60fps = 16.67ms
                    queue_draw();
                    last_redraw_time = now;
                }
            }
        }else if(current_tool == "eraser" && is_erasing){
            // Erase items on contact and add to preview
            update_eraser_collision(x, y);
            queue_draw(); // Trigger redraw
        }
        else if (is_drawing_rectangle || is_drawing_circle) {
            // Update current mouse position for rectangle preview
            current_mouse_pos = Point(x, y);
            queue_draw(); // Trigger redraw to show preview
        }
        else if (current_tool == "select") {
            if (is_selecting) {
                current_mouse_pos = Point(x, y);
                queue_draw();
            } else if (is_moving_selection) {
                double dx = x - selection_start.x;
                double dy = y - selection_start.y;
                move_selected_objects(dx, dy);
                selection_start = Point(x, y);
                queue_draw();
            }
        }
    });
    
    // Mouse release - finish drawing
    click_gesture->signal_released().connect([this](int n_press, double x, double y){
        if (is_drawing) {
            current_stroke.add_point(x, y);
            // Complete stroke and render to background surface (like SVG layer)
            current_stroke.complete_stroke();
            render_stroke_to_background(current_stroke);
            
            // Optional: keep in vector for other features (eraser, selection, etc.)
            completed_strokes.push_back(current_stroke);
            
            current_stroke = Stroke(current_pen_width, current_pen_color); // Reset with current settings
            if(current_tool == "pen" && is_drawing == true)is_drawing = false;
            queue_draw();
        }
        else if (is_drawing_rectangle) {
            // Calculate rectangle dimensions
            double width = x - rectangle_start.x;
            double height = y - rectangle_start.y;
            
            // Add rectangle to current rectangle object
            current_rectangle.add_rect(rectangle_start.x, rectangle_start.y, width, height, default_rectangle_color);
            
            // Render to background surface before adding to vector
            render_rectangle_to_background(current_rectangle);
            completed_rectangles.push_back(current_rectangle);
            
            // Clear current rectangle
            current_rectangle = Rectangle();
            is_drawing_rectangle = false;
            queue_draw();
        }
        else if(is_drawing_circle){
            double r = sqrt(pow(x - circle_start.x, 2) + pow(y - circle_start.y, 2));

            current_circle.add_circle(circle_start.x, circle_start.y, r, default_circle_color);
            
            // Render to background surface before adding to vector
            render_circle_to_background(current_circle);
            completed_circles.push_back(current_circle);

            current_circle = Circle();
            is_drawing_circle = false;
            queue_draw();
        }
        else if (is_erasing) {
            // Stop erasing mode and clear preview vectors
            current_eraser.stroke_to_erase.clear();
            current_eraser.rectangle_to_erase.clear();
            current_eraser.circle_to_erase.clear();
            is_erasing = false;
            queue_draw();
        }
        else if (current_tool == "select") {
            if (is_selecting) {
                // Complete selection rectangle
                select_objects_in_rectangle(selection_start.x, selection_start.y, x, y);
                is_selecting = false;
            } else if (is_moving_selection) {
                // Complete move operation
                is_moving_selection = false;
            }
            queue_draw();
        }
    });
    
    add_controller(motion_controller);
    add_controller(click_gesture);
}

void CairoDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    // Ensure background surface matches current size
    if (!background_surface || background_surface->get_width() != width || background_surface->get_height() != height) {
        initialize_background_surface(width, height);
        rebuild_background_surface(); // Re-render all objects to new surface
    }
    
    // Clear background
    cr->set_source_rgba(0, 0, 0, 0); // Transparent background
    cr->paint();
    
    // PERFORMANCE FIX: Blit cached background surface (contains all completed objects)
    if (background_surface) {
        cr->set_source(background_surface, 0, 0);
        cr->paint();
    }
    
    // Draw selection highlights on live layer (overlay on top of background)
    draw_selection_highlights(cr);
    
    // Draw selection rectangle if selecting
    if (current_tool == "select" && is_selecting) {
        draw_selection_rectangle(cr, selection_start.x, selection_start.y, current_mouse_pos.x, current_mouse_pos.y);
    }
    
    // Draw unified drawable objects (for future use)
    for (const auto& obj : drawable_objects) {
        if (obj->is_selected) {
            obj->draw_selection_handles(cr);
        }
    }
    
    // Draw current stroke if drawing - points are already smooth in real-time
    if (is_drawing && !current_stroke.points.empty()) {
        draw_stroke(cr, current_stroke);
    }
    
    // Draw rectangle preview if drawing rectangle
    if (is_drawing_rectangle) {
        draw_rectangle_preview(cr, rectangle_start.x, rectangle_start.y, current_mouse_pos.x, current_mouse_pos.y);
    }

    // Draw circle preview if drawing circle
    if (is_drawing_circle){
        double r = sqrt(pow(current_mouse_pos.x - circle_start.x, 2) + pow(current_mouse_pos.y - circle_start.y, 2));
        draw_circle_preview(cr, circle_start.x, circle_start.y, r);
    }
    
    // Draw erasing preview if actively erasing
    if (is_erasing) {
        for (const auto& stroke : current_eraser.stroke_to_erase) {
            draw_erasing_preview(cr, stroke);
        }
        for (const auto& rect : current_eraser.rectangle_to_erase) {
            // Draw rectangle with 50% transparency
            cr->set_source_rgba(rect.color.r, rect.color.g, rect.color.b, 0.5);
            cr->set_line_width(2.0);
            cr->rectangle(rect.x, rect.y, rect.width, rect.height);
            cr->stroke();
        }

        for (const auto& c : current_eraser.circle_to_erase){
            // Draw circle with 50% transparency
            cr->set_source_rgba(c.color.r, c.color.g, c.color.b, 0.5);
            cr->set_line_width(2.0);
            cr->arc(c.x, c.y, c.r, 0, 2 * M_PI);
            cr->stroke();
        }
    }
    
}

void CairoDrawingArea::draw_stroke(const Cairo::RefPtr<Cairo::Context>& cr, const Stroke& stroke) {
    if (stroke.points.size() < 2) return;
    
    // Use smooth stroke rendering for better quality
    draw_smooth_stroke(cr, stroke);
}

void CairoDrawingArea::draw_current_stroke_simple(const Cairo::RefPtr<Cairo::Context>& cr, const Stroke& stroke) {
    if (stroke.points.size() < 2) return;
    
    // Set stroke properties with antialiasing
    cr->set_source_rgba(stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
    cr->set_line_width(stroke.width);
    cr->set_line_cap(Cairo::Context::LineCap::ROUND);
    cr->set_line_join(Cairo::Context::LineJoin::ROUND);
    
    // Start path at first point
    cr->move_to(stroke.points[0].x, stroke.points[0].y);
    
    // Draw simple lines connecting all points (no smoothing for performance)
    for (size_t i = 1; i < stroke.points.size(); i++) {
        cr->line_to(stroke.points[i].x, stroke.points[i].y);
    }
    
    // Stroke the path
    cr->stroke();
}

void CairoDrawingArea::draw_rectangle(const Cairo::RefPtr<Cairo::Context>& cr, const Rectangle& rectangle) {
    for (const auto& rect : rectangle.rects) {
        cr->set_source_rgb(rect.color.r, rect.color.g, rect.color.b);
        cr->set_line_width(2.0);
        
        // Draw rectangle outline
        cr->rectangle(rect.x, rect.y, rect.width, rect.height);
        cr->stroke();
    }
}

void CairoDrawingArea::draw_circle(const Cairo::RefPtr<Cairo::Context>& cr, const Circle& circle){
    for(const auto& c : circle.circles){
        cr->set_source_rgb(c.color.r, c.color.g, c.color.b);
        cr->set_line_width(2.0);

        // Draw circle
        cr->arc(c.x, c.y, c.r, 0, 2 * M_PI);
        cr->stroke();
    }
}

void CairoDrawingArea::draw_circle_preview(const Cairo::RefPtr<Cairo::Context>& cr, double start_x, double start_y, double r){
    cr->set_source_rgba(0.0, 0.0, 0.8, 0.5);
    cr->set_line_width(1.0);

    // Draw circle preview
    cr->arc(start_x, start_y, r, 0, 2 * M_PI);
    cr->stroke();
}

void CairoDrawingArea::draw_rectangle_preview(const Cairo::RefPtr<Cairo::Context>& cr, double start_x, double start_y, double end_x, double end_y) {
    cr->set_source_rgba(0.0, 0.0, 0.8, 0.5); // Semi-transparent blue
    cr->set_line_width(1.0);
    
    double width = end_x - start_x;
    double height = end_y - start_y;
    
    cr->rectangle(start_x, start_y, width, height);
    cr->stroke();
}

void CairoDrawingArea::draw_smooth_stroke(const Cairo::RefPtr<Cairo::Context>& cr, const Stroke& stroke) {
    if (stroke.points.size() < 2) return;
    
    // Points now contain calculated smooth points directly
    // Set stroke properties with antialiasing
    cr->set_source_rgba(stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
    cr->set_line_width(stroke.width);
    cr->set_line_cap(Cairo::Context::LineCap::ROUND);
    cr->set_line_join(Cairo::Context::LineJoin::ROUND);
    // Cairo automatically provides good antialiasing by default
    
    // Start path at first point
    cr->move_to(stroke.points[0].x, stroke.points[0].y);
    
    // Draw smooth lines connecting all interpolated points
    for (size_t i = 1; i < stroke.points.size(); i++) {
        cr->line_to(stroke.points[i].x, stroke.points[i].y);
    }
    
    // Stroke the path
    cr->stroke();
}

std::vector<Point> CairoDrawingArea::simplify_stroke(const std::vector<Point>& points, double tolerance) {
    if (points.size() <= 2) return points;
    
    std::vector<Point> simplified;
    simplified.push_back(points[0]); // Always keep first point
    
    for (size_t i = 1; i < points.size() - 1; i++) {
        const Point& prev = simplified.back();
        const Point& current = points[i];
        const Point& next = points[i + 1];
        
        // Calculate distance from current point to line between prev and next
        double dist = point_distance(prev, current);
        
        // Keep point if it's far enough from the previous point
        if (dist >= tolerance) {
            simplified.push_back(current);
        }
    }
    
    simplified.push_back(points.back()); // Always keep last point
    return simplified;
}

std::vector<Point> CairoDrawingArea::ultra_smooth_stroke(const std::vector<Point>& points) {
    if (points.size() < 2) return points;
    
    // Step 1: Light simplification to remove micro-jitter
    std::vector<Point> simplified = simplify_stroke(points, 0.5);
    
    // Step 2: Heavy Catmull-Rom interpolation for ultra-smooth curves
    std::vector<Point> ultra_smooth = interpolate_catmull_rom(simplified, 12);
    
    return ultra_smooth;
}

std::vector<Point> CairoDrawingArea::interpolate_catmull_rom(const std::vector<Point>& points, int segments_per_curve) {
    if (points.size() < 2) return points;
    if (points.size() == 2) return points;
    
    std::vector<Point> interpolated;
    interpolated.reserve(points.size() * segments_per_curve);
    
    // Add first point
    interpolated.push_back(points[0]);
    
    // Interpolate between each consecutive pair of points
    for (size_t i = 0; i < points.size() - 1; i++) {
        // Get the four control points for Catmull-Rom spline
        Point p0 = (i > 0) ? points[i-1] : points[i];
        Point p1 = points[i];
        Point p2 = points[i+1];
        Point p3 = (i+2 < points.size()) ? points[i+2] : points[i+1];
        
        // Generate many interpolated points for super smooth curves
        for (int j = 1; j <= segments_per_curve; j++) {
            double t = (double)j / segments_per_curve;
            Point interpolated_point = catmull_rom_point(p0, p1, p2, p3, t);
            interpolated.push_back(interpolated_point);
        }
    }
    
    return interpolated;
}

Point CairoDrawingArea::catmull_rom_point(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t) {
    double t2 = t * t;
    double t3 = t2 * t;
    
    // Catmull-Rom spline formula
    double x = 0.5 * ((2.0 * p1.x) +
                     (-p0.x + p2.x) * t +
                     (2.0 * p0.x - 5.0 * p1.x + 4.0 * p2.x - p3.x) * t2 +
                     (-p0.x + 3.0 * p1.x - 3.0 * p2.x + p3.x) * t3);
    
    double y = 0.5 * ((2.0 * p1.y) +
                     (-p0.y + p2.y) * t +
                     (2.0 * p0.y - 5.0 * p1.y + 4.0 * p2.y - p3.y) * t2 +
                     (-p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y) * t3);
    
    return Point(x, y);
}

double CairoDrawingArea::point_distance(const Point& p1, const Point& p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    return sqrt(dx * dx + dy * dy);
}

// Public interface methods
void CairoDrawingArea::clear_canvas() {
    completed_strokes.clear();
    current_stroke = Stroke(current_pen_width, current_pen_color);
    is_drawing = false;
    queue_draw();
}

void CairoDrawingArea::undo() {
    if (!completed_strokes.empty()) {
        completed_strokes.pop_back();
        queue_draw();
    }
}

void CairoDrawingArea::set_stroke_width(double width) {
    current_pen_width = width;
    current_stroke.width = width;
}

void CairoDrawingArea::set_stroke_color(const Color& color) {
    current_pen_color = color;
    current_stroke.color = color;
}

void CairoDrawingArea::set_stroke_opacity(double opacity) {
    current_pen_color.a = opacity;
    current_stroke.color.a = opacity;
}

void CairoDrawingArea::set_rectangle_color(const Color& color) {
    default_rectangle_color = color;
}

void CairoDrawingArea::draw_erasing_preview(const Cairo::RefPtr<Cairo::Context>& cr, const Stroke& stroke) {
    if (stroke.points.size() < 2) return;
    
    // Points now contain calculated smooth points directly
    // Draw stroke with 50% transparency to show it will be erased
    cr->set_source_rgba(stroke.color.r, stroke.color.g, stroke.color.b, 0.5);
    cr->set_line_width(stroke.width);
    cr->set_line_cap(Cairo::Context::LineCap::ROUND);
    cr->set_line_join(Cairo::Context::LineJoin::ROUND);
    
    // Draw the smooth stroke path
    cr->move_to(stroke.points[0].x, stroke.points[0].y);
    for (size_t i = 1; i < stroke.points.size(); i++) {
        cr->line_to(stroke.points[i].x, stroke.points[i].y);
    }
    cr->stroke();
}

bool CairoDrawingArea::is_stroke_in_eraser_radius(const Stroke& stroke, double eraser_x, double eraser_y, double radius) {
    // Points now contain calculated smooth points directly
    // Check if any point in the smooth stroke is within the eraser radius
    for (const auto& point : stroke.points) {
        double distance = point_distance(Point(eraser_x, eraser_y), point);
        if (distance <= radius) {
            return true;
        }
    }
    return false;
}

bool CairoDrawingArea::is_rect_in_eraser_radius(const Rect& rect, double eraser_x, double eraser_y, double radius) {
    // Check if eraser circle intersects with rectangle
    // Find closest point on rectangle to eraser center
    double closest_x = std::max(rect.x, std::min(eraser_x, rect.x + rect.width));
    double closest_y = std::max(rect.y, std::min(eraser_y, rect.y + rect.height));
    
    double distance = point_distance(Point(eraser_x, eraser_y), Point(closest_x, closest_y));
    return distance <= radius;
}

bool CairoDrawingArea::is_circle_in_eraser_radius(const Circle_Data& circle, double eraser_x, double eraser_y, double radius) {
    // Check if eraser circle intersects with the drawn circle
    // Calculate distance between eraser center and circle center
    double distance = point_distance(Point(eraser_x, eraser_y), Point(circle.x, circle.y));
    
    // Circles intersect if distance between centers is less than sum of radii
    return distance <= (radius + circle.r);
}

void CairoDrawingArea::update_eraser_collision(double x, double y) {
    const double eraser_radius = 10.0; // Default eraser radius
    
    // Move strokes from main vector to preview vector on collision
    for (auto it = completed_strokes.begin(); it != completed_strokes.end();) {
        if (is_stroke_in_eraser_radius(*it, x, y, eraser_radius)) {
            // Check if this stroke is already in the preview vector
            bool already_in_preview = false;
            for (const auto& preview_stroke : current_eraser.stroke_to_erase) {
                if (&(*it) == &preview_stroke) {
                    already_in_preview = true;
                    break;
                }
            }
            
            if (!already_in_preview) {
                // Move stroke to preview vector
                current_eraser.stroke_to_erase.push_back(*it);
                // Remove from main vector
                it = completed_strokes.erase(it);
                // IMPORTANT: Rebuild background surface after removing stroke
                rebuild_background_surface();
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
    
    // Move rectangles from main vector to preview vector on collision
    for (auto rect_it = completed_rectangles.begin(); rect_it != completed_rectangles.end();) {
        bool rectangle_moved = false;
        
        // Check each rect in the rectangle
        for (auto& rect : rect_it->rects) {
            if (is_rect_in_eraser_radius(rect, x, y, eraser_radius)) {
                // Check if this rectangle is already in preview
                bool already_in_preview = false;
                for (const auto& preview_rect : current_eraser.rectangle_to_erase) {
                    if (&rect == &preview_rect) {
                        already_in_preview = true;
                        break;
                    }
                }
                
                if (!already_in_preview) {
                    // Move all rects from this rectangle to preview
                    for (const auto& r : rect_it->rects) {
                        current_eraser.rectangle_to_erase.push_back(r);
                    }
                    // Remove from main vector
                    rect_it = completed_rectangles.erase(rect_it);
                    rectangle_moved = true;
                    // IMPORTANT: Rebuild background surface after removing rectangle
                    rebuild_background_surface();
                }
                break; // Break inner loop since we processed the whole rectangle
            }
        }
        
        if (!rectangle_moved) {
            ++rect_it; // Only increment if we didn't move
        }
    }
    
    // Move circles from main vector to preview vector on collision
    for (auto circle_it = completed_circles.begin(); circle_it != completed_circles.end();) {
        bool circle_moved = false;
        
        // Check each circle in the Circle object
        for (auto& circle : circle_it->circles) {
            if (is_circle_in_eraser_radius(circle, x, y, eraser_radius)) {
                // Check if this circle is already in preview
                bool already_in_preview = false;
                for (const auto& preview_circle : current_eraser.circle_to_erase) {
                    if (&circle == &preview_circle) {
                        already_in_preview = true;
                        break;
                    }
                }
                
                if (!already_in_preview) {
                    // Move all circles from this Circle object to preview
                    for (const auto& c : circle_it->circles) {
                        current_eraser.circle_to_erase.push_back(c);
                    }
                    // Remove from main vector
                    circle_it = completed_circles.erase(circle_it);
                    circle_moved = true;
                    // IMPORTANT: Rebuild background surface after removing circle
                    rebuild_background_surface();
                }
                break; // Break inner loop since we processed the whole Circle object
            }
        }
        
        if (!circle_moved) {
            ++circle_it; // Only increment if we didn't move
        }
    }
}

void CairoDrawingArea::update_eraser_preview(double x, double y) {
    const double eraser_radius = 20.0; // Default eraser radius
    
    // Clear previous preview selections
    current_eraser.stroke_to_erase.clear();
    current_eraser.rectangle_to_erase.clear();
    current_eraser.circle_to_erase.clear();
    
    // Check collision with strokes for preview (don't erase yet)
    for (const auto& stroke : completed_strokes) {
        if (is_stroke_in_eraser_radius(stroke, x, y, eraser_radius)) {
            current_eraser.stroke_to_erase.push_back(stroke);
        }
    }
    
    // Check collision with rectangles for preview (don't erase yet)
    for (const auto& rectangle : completed_rectangles) {
        for (const auto& rect : rectangle.rects) {
            if (is_rect_in_eraser_radius(rect, x, y, eraser_radius)) {
                current_eraser.rectangle_to_erase.push_back(rect);
            }
        }
    }
    
    // Check collision with circles for preview (don't erase yet)
    for (const auto& circle : completed_circles) {
        for (const auto& c : circle.circles) {
            if (is_circle_in_eraser_radius(c, x, y, eraser_radius)) {
                current_eraser.circle_to_erase.push_back(c);
            }
        }
    }
}

void CairoDrawingArea::set_drawing_state(std::string state){
    current_tool = state;
    set_current_cursor();
}

void CairoDrawingArea::set_current_cursor() {
    Glib::RefPtr<Gdk::Cursor> cursor;
    
    if (current_tool == "pen") {
        cursor = Gdk::Cursor::create("crosshair");
    } else if (current_tool == "eraser") {
        cursor = Gdk::Cursor::create("pointer");
    } else if (current_tool == "text") {
        cursor = Gdk::Cursor::create("text");
    } else if (current_tool == "select") {
        cursor = Gdk::Cursor::create("pointer");
    } else {
        cursor = Gdk::Cursor::create("crosshair");
    }
    
    set_cursor(cursor);
}

void CairoDrawingArea::clear_selection() {
    clear_all_selections();
    queue_draw();
}

// ===== UNIFIED OBJECT SYSTEM IMPLEMENTATIONS =====

// DrawableObject base class methods
HandlePosition DrawableObject::get_handle_at_point(double x, double y) const {
    BoundingBox bbox = get_bounding_box();
    const double handle_size = 8.0;
    const double half_handle = handle_size / 2.0;
    
    // Check corner handles first
    if (std::abs(x - bbox.x) <= half_handle && std::abs(y - bbox.y) <= half_handle)
        return HandlePosition::TOP_LEFT;
    if (std::abs(x - (bbox.x + bbox.width)) <= half_handle && std::abs(y - bbox.y) <= half_handle)
        return HandlePosition::TOP_RIGHT;
    if (std::abs(x - bbox.x) <= half_handle && std::abs(y - (bbox.y + bbox.height)) <= half_handle)
        return HandlePosition::BOTTOM_LEFT;
    if (std::abs(x - (bbox.x + bbox.width)) <= half_handle && std::abs(y - (bbox.y + bbox.height)) <= half_handle)
        return HandlePosition::BOTTOM_RIGHT;
    
    // Check edge handles
    double mid_x = bbox.x + bbox.width / 2.0;
    double mid_y = bbox.y + bbox.height / 2.0;
    
    if (std::abs(x - mid_x) <= half_handle && std::abs(y - bbox.y) <= half_handle)
        return HandlePosition::TOP;
    if (std::abs(x - mid_x) <= half_handle && std::abs(y - (bbox.y + bbox.height)) <= half_handle)
        return HandlePosition::BOTTOM;
    if (std::abs(x - bbox.x) <= half_handle && std::abs(y - mid_y) <= half_handle)
        return HandlePosition::LEFT;
    if (std::abs(x - (bbox.x + bbox.width)) <= half_handle && std::abs(y - mid_y) <= half_handle)
        return HandlePosition::RIGHT;
    
    return HandlePosition::NONE;
}

void DrawableObject::draw_selection_handles(const Cairo::RefPtr<Cairo::Context>& cr) const {
    if (!is_selected) return;
    
    BoundingBox bbox = get_bounding_box();
    const double handle_size = 8.0;
    const double half_handle = handle_size / 2.0;
    
    cr->set_source_rgb(0.2, 0.6, 1.0); // Blue handles
    cr->set_line_width(1.0);
    
    // Draw corner handles
    std::vector<std::pair<double, double>> handle_positions = {
        {bbox.x, bbox.y}, // TOP_LEFT
        {bbox.x + bbox.width, bbox.y}, // TOP_RIGHT
        {bbox.x, bbox.y + bbox.height}, // BOTTOM_LEFT
        {bbox.x + bbox.width, bbox.y + bbox.height}, // BOTTOM_RIGHT
        {bbox.x + bbox.width/2, bbox.y}, // TOP
        {bbox.x + bbox.width/2, bbox.y + bbox.height}, // BOTTOM
        {bbox.x, bbox.y + bbox.height/2}, // LEFT
        {bbox.x + bbox.width, bbox.y + bbox.height/2} // RIGHT
    };
    
    for (const auto& pos : handle_positions) {
        cr->rectangle(pos.first - half_handle, pos.second - half_handle, handle_size, handle_size);
        cr->fill_preserve();
        cr->set_source_rgb(0.0, 0.0, 0.0);
        cr->stroke();
        cr->set_source_rgb(0.2, 0.6, 1.0);
    }
}

// StrokeObject implementation
void StrokeObject::draw(const Cairo::RefPtr<Cairo::Context>& cr) const {
    if (stroke.points.size() < 2) return;
    
    cr->set_source_rgba(stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
    cr->set_line_width(stroke.width);
    cr->set_line_cap(Cairo::Context::LineCap::ROUND);
    cr->set_line_join(Cairo::Context::LineJoin::ROUND);
    
    cr->move_to(stroke.points[0].x, stroke.points[0].y);
    for (size_t i = 1; i < stroke.points.size(); i++) {
        cr->line_to(stroke.points[i].x, stroke.points[i].y);
    }
    cr->stroke();
}

BoundingBox StrokeObject::get_bounding_box() const {
    if (stroke.points.empty()) return BoundingBox();
    
    double min_x = stroke.points[0].x, max_x = stroke.points[0].x;
    double min_y = stroke.points[0].y, max_y = stroke.points[0].y;
    
    for (const auto& point : stroke.points) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    
    // Add stroke width padding
    double padding = stroke.width / 2.0;
    return BoundingBox(min_x - padding, min_y - padding, 
                      max_x - min_x + 2*padding, max_y - min_y + 2*padding);
}

bool StrokeObject::hit_test(double x, double y) const {
    const double tolerance = stroke.width / 2.0 + 2.0;
    
    for (const auto& point : stroke.points) {
        double distance = sqrt((x - point.x)*(x - point.x) + (y - point.y)*(y - point.y));
        if (distance <= tolerance) return true;
    }
    return false;
}

void StrokeObject::translate(double dx, double dy) {
    for (auto& point : stroke.points) {
        point.x += dx;
        point.y += dy;
    }
}

void StrokeObject::scale(double scale_x, double scale_y, double origin_x, double origin_y) {
    for (auto& point : stroke.points) {
        point.x = origin_x + (point.x - origin_x) * scale_x;
        point.y = origin_y + (point.y - origin_y) * scale_y;
    }
    stroke.width *= std::min(scale_x, scale_y); // Scale line width proportionally
}

// RectangleObject implementation
void RectangleObject::draw(const Cairo::RefPtr<Cairo::Context>& cr) const {
    cr->set_source_rgb(rect.color.r, rect.color.g, rect.color.b);
    cr->set_line_width(2.0);
    cr->rectangle(rect.x, rect.y, rect.width, rect.height);
    cr->stroke();
}

BoundingBox RectangleObject::get_bounding_box() const {
    return BoundingBox(rect.x, rect.y, rect.width, rect.height);
}

bool RectangleObject::hit_test(double x, double y) const {
    return x >= rect.x && x <= rect.x + rect.width && 
           y >= rect.y && y <= rect.y + rect.height;
}

void RectangleObject::translate(double dx, double dy) {
    rect.x += dx;
    rect.y += dy;
}

void RectangleObject::scale(double scale_x, double scale_y, double origin_x, double origin_y) {
    rect.x = origin_x + (rect.x - origin_x) * scale_x;
    rect.y = origin_y + (rect.y - origin_y) * scale_y;
    rect.width *= scale_x;
    rect.height *= scale_y;
}

// CircleObject implementation
void CircleObject::draw(const Cairo::RefPtr<Cairo::Context>& cr) const {
    cr->set_source_rgb(circle.color.r, circle.color.g, circle.color.b);
    cr->set_line_width(2.0);
    cr->arc(circle.x, circle.y, circle.r, 0, 2 * M_PI);
    cr->stroke();
}

BoundingBox CircleObject::get_bounding_box() const {
    return BoundingBox(circle.x - circle.r, circle.y - circle.r, 
                      2 * circle.r, 2 * circle.r);
}

bool CircleObject::hit_test(double x, double y) const {
    double distance = sqrt((x - circle.x)*(x - circle.x) + (y - circle.y)*(y - circle.y));
    return distance <= circle.r;
}

void CircleObject::translate(double dx, double dy) {
    circle.x += dx;
    circle.y += dy;
}

void CircleObject::scale(double scale_x, double scale_y, double origin_x, double origin_y) {
    circle.x = origin_x + (circle.x - origin_x) * scale_x;
    circle.y = origin_y + (circle.y - origin_y) * scale_y;
    circle.r *= std::min(scale_x, scale_y); // Keep circle round
}

// SelectionManager implementation
void SelectionManager::clear_selection() {
    for (auto& obj : selected_objects) {
        obj->is_selected = false;
    }
    selected_objects.clear();
}

void SelectionManager::add_to_selection(std::shared_ptr<DrawableObject> obj) {
    if (!is_selected(obj)) {
        selected_objects.push_back(obj);
        obj->is_selected = true;
    }
}

void SelectionManager::remove_from_selection(std::shared_ptr<DrawableObject> obj) {
    auto it = std::find(selected_objects.begin(), selected_objects.end(), obj);
    if (it != selected_objects.end()) {
        (*it)->is_selected = false;
        selected_objects.erase(it);
    }
}

bool SelectionManager::is_selected(std::shared_ptr<DrawableObject> obj) const {
    return std::find(selected_objects.begin(), selected_objects.end(), obj) != selected_objects.end();
}

BoundingBox SelectionManager::get_selection_bounds() const {
    if (selected_objects.empty()) return BoundingBox();
    
    BoundingBox first_bbox = selected_objects[0]->get_bounding_box();
    double min_x = first_bbox.x, max_x = first_bbox.x + first_bbox.width;
    double min_y = first_bbox.y, max_y = first_bbox.y + first_bbox.height;
    
    for (const auto& obj : selected_objects) {
        BoundingBox bbox = obj->get_bounding_box();
        min_x = std::min(min_x, bbox.x);
        max_x = std::max(max_x, bbox.x + bbox.width);
        min_y = std::min(min_y, bbox.y);
        max_y = std::max(max_y, bbox.y + bbox.height);
    }
    
    return BoundingBox(min_x, min_y, max_x - min_x, max_y - min_y);
}

void SelectionManager::move_selection(double dx, double dy) {
    for (auto& obj : selected_objects) {
        obj->translate(dx, dy);
    }
}

void SelectionManager::scale_selection(double scale_x, double scale_y, double origin_x, double origin_y) {
    for (auto& obj : selected_objects) {
        obj->scale(scale_x, scale_y, origin_x, origin_y);
    }
}

// CairoDrawingArea selection system methods
std::shared_ptr<DrawableObject> CairoDrawingArea::find_object_at_point(double x, double y) {
    // Iterate in reverse order to check top-most objects first
    for (auto it = drawable_objects.rbegin(); it != drawable_objects.rend(); ++it) {
        if ((*it)->hit_test(x, y)) {
            return *it;
        }
    }
    return nullptr;
}

HandlePosition CairoDrawingArea::find_handle_at_point(double x, double y) {
    for (const auto& obj : drawable_objects) {
        if (obj->is_selected) {
            HandlePosition handle = obj->get_handle_at_point(x, y);
            if (handle != HandlePosition::NONE) {
                return handle;
            }
        }
    }
    return HandlePosition::NONE;
}

void CairoDrawingArea::update_selection(double x, double y, bool multi_select) {
    auto clicked_obj = find_object_at_point(x, y);
    
    if (!multi_select && !clicked_obj) {
        // Clear selection if clicking on empty space
        selection_manager.clear_selection();
    } else if (clicked_obj) {
        if (multi_select) {
            // Toggle selection for multi-select
            if (selection_manager.is_selected(clicked_obj)) {
                selection_manager.remove_from_selection(clicked_obj);
            } else {
                selection_manager.add_to_selection(clicked_obj);
            }
        } else {
            // Single selection
            selection_manager.clear_selection();
            selection_manager.add_to_selection(clicked_obj);
        }
    }
}

void CairoDrawingArea::start_move_operation(double x, double y) {
    if (selection_manager.has_selection()) {
        is_moving = true;
        selection_start = Point(x, y);
    }
}

void CairoDrawingArea::start_resize_operation(double x, double y, HandlePosition handle) {
    if (selection_manager.has_selection() && handle != HandlePosition::NONE) {
        is_resizing = true;
        current_handle = handle;
        selection_start = Point(x, y);
    }
}

void CairoDrawingArea::perform_move(double x, double y) {
    if (is_moving) {
        double dx = x - selection_start.x;
        double dy = y - selection_start.y;
        
        selection_manager.move_selection(dx, dy);
        selection_start = Point(x, y);
    }
}

void CairoDrawingArea::perform_resize(double x, double y) {
    if (is_resizing && current_handle != HandlePosition::NONE) {
        BoundingBox bounds = selection_manager.get_selection_bounds();
        
        // Calculate scale factors based on handle position and mouse movement
        double scale_x = 1.0, scale_y = 1.0;
        double origin_x = bounds.x + bounds.width / 2.0;
        double origin_y = bounds.y + bounds.height / 2.0;
        
        double dx = x - selection_start.x;
        double dy = y - selection_start.y;
        
        switch (current_handle) {
            case HandlePosition::TOP_LEFT:
                scale_x = (bounds.width - dx) / bounds.width;
                scale_y = (bounds.height - dy) / bounds.height;
                origin_x = bounds.x + bounds.width;
                origin_y = bounds.y + bounds.height;
                break;
            case HandlePosition::TOP_RIGHT:
                scale_x = (bounds.width + dx) / bounds.width;
                scale_y = (bounds.height - dy) / bounds.height;
                origin_x = bounds.x;
                origin_y = bounds.y + bounds.height;
                break;
            case HandlePosition::BOTTOM_LEFT:
                scale_x = (bounds.width - dx) / bounds.width;
                scale_y = (bounds.height + dy) / bounds.height;
                origin_x = bounds.x + bounds.width;
                origin_y = bounds.y;
                break;
            case HandlePosition::BOTTOM_RIGHT:
                scale_x = (bounds.width + dx) / bounds.width;
                scale_y = (bounds.height + dy) / bounds.height;
                origin_x = bounds.x;
                origin_y = bounds.y;
                break;
            case HandlePosition::LEFT:
                scale_x = (bounds.width - dx) / bounds.width;
                origin_x = bounds.x + bounds.width;
                break;
            case HandlePosition::RIGHT:
                scale_x = (bounds.width + dx) / bounds.width;
                origin_x = bounds.x;
                break;
            case HandlePosition::TOP:
                scale_y = (bounds.height - dy) / bounds.height;
                origin_y = bounds.y + bounds.height;
                break;
            case HandlePosition::BOTTOM:
                scale_y = (bounds.height + dy) / bounds.height;
                origin_y = bounds.y;
                break;
            default:
                break;
        }
        
        // Prevent negative scaling
        if (scale_x > 0.1 && scale_y > 0.1) {
            selection_manager.scale_selection(scale_x, scale_y, origin_x, origin_y);
            selection_start = Point(x, y);
        }
    }
}

void CairoDrawingArea::add_drawable_object(std::shared_ptr<DrawableObject> obj) {
    drawable_objects.push_back(obj);
}

void CairoDrawingArea::remove_drawable_object(std::shared_ptr<DrawableObject> obj) {
    auto it = std::find(drawable_objects.begin(), drawable_objects.end(), obj);
    if (it != drawable_objects.end()) {
        drawable_objects.erase(it);
    }
}

// Selection functions implementation
void CairoDrawingArea::clear_all_selections() {
    selected_stroke_indices.clear();
    selected_rectangle_indices.clear();
    selected_circle_indices.clear();
}

void CairoDrawingArea::select_objects_in_rectangle(double x1, double y1, double x2, double y2) {
    // Ensure correct rectangle bounds
    double min_x = std::min(x1, x2);
    double max_x = std::max(x1, x2);
    double min_y = std::min(y1, y2);
    double max_y = std::max(y1, y2);
    
    // Select strokes that intersect with selection rectangle
    for (int i = 0; i < completed_strokes.size(); i++) {
        const auto& stroke = completed_strokes[i];
        bool intersects = false;
        
        for (const auto& point : stroke.points) {
            if (point.x >= min_x && point.x <= max_x && point.y >= min_y && point.y <= max_y) {
                intersects = true;
                break;
            }
        }
        
        if (intersects) {
            selected_stroke_indices.push_back(i);
        }
    }
    
    // Select rectangles that intersect with selection rectangle
    for (int i = 0; i < completed_rectangles.size(); i++) {
        const auto& rectangle = completed_rectangles[i];
        bool intersects = false;
        
        for (const auto& rect : rectangle.rects) {
            if (!(rect.x + rect.width < min_x || rect.x > max_x || 
                  rect.y + rect.height < min_y || rect.y > max_y)) {
                intersects = true;
                break;
            }
        }
        
        if (intersects) {
            selected_rectangle_indices.push_back(i);
        }
    }
    
    // Select circles that intersect with selection rectangle
    for (int i = 0; i < completed_circles.size(); i++) {
        const auto& circle = completed_circles[i];
        bool intersects = false;
        
        for (const auto& c : circle.circles) {
            if (!(c.x + c.r < min_x || c.x - c.r > max_x || 
                  c.y + c.r < min_y || c.y - c.r > max_y)) {
                intersects = true;
                break;
            }
        }
        
        if (intersects) {
            selected_circle_indices.push_back(i);
        }
    }
}

void CairoDrawingArea::move_selected_objects(double dx, double dy) {
    // Move selected strokes
    for (int idx : selected_stroke_indices) {
        if (idx < completed_strokes.size()) {
            for (auto& point : completed_strokes[idx].points) {
                point.x += dx;
                point.y += dy;
            }
        }
    }
    
    // Move selected rectangles
    for (int idx : selected_rectangle_indices) {
        if (idx < completed_rectangles.size()) {
            for (auto& rect : completed_rectangles[idx].rects) {
                rect.x += dx;
                rect.y += dy;
            }
        }
    }
    
    // Move selected circles
    for (int idx : selected_circle_indices) {
        if (idx < completed_circles.size()) {
            for (auto& circle : completed_circles[idx].circles) {
                circle.x += dx;
                circle.y += dy;
            }
        }
    }
    
    // CRITICAL: Rebuild background surface after moving objects
    rebuild_background_surface();
}

void CairoDrawingArea::draw_selection_rectangle(const Cairo::RefPtr<Cairo::Context>& cr, double x1, double y1, double x2, double y2) {
    cr->set_source_rgba(0.2, 0.4, 0.8, 0.3); // Semi-transparent blue
    cr->rectangle(std::min(x1, x2), std::min(y1, y2), std::abs(x2 - x1), std::abs(y2 - y1));
    cr->fill_preserve();
    
    cr->set_source_rgba(0.2, 0.4, 0.8, 0.8); // Darker blue border
    cr->set_line_width(1.0);
    cr->stroke();
}

void CairoDrawingArea::draw_selection_highlights(const Cairo::RefPtr<Cairo::Context>& cr) {
    // Highlight selected strokes
    for (int idx : selected_stroke_indices) {
        if (idx < completed_strokes.size()) {
            const auto& stroke = completed_strokes[idx];
            cr->set_source_rgba(0.8, 0.4, 0.2, 0.6); // Orange highlight
            cr->set_line_width(stroke.width + 4.0);
            cr->set_line_cap(Cairo::Context::LineCap::ROUND);
            cr->set_line_join(Cairo::Context::LineJoin::ROUND);
            
            if (stroke.points.size() >= 2) {
                cr->move_to(stroke.points[0].x, stroke.points[0].y);
                for (size_t i = 1; i < stroke.points.size(); i++) {
                    cr->line_to(stroke.points[i].x, stroke.points[i].y);
                }
                cr->stroke();
            }
        }
    }
    
    // Highlight selected rectangles
    for (int idx : selected_rectangle_indices) {
        if (idx < completed_rectangles.size()) {
            for (const auto& rect : completed_rectangles[idx].rects) {
                cr->set_source_rgba(0.8, 0.4, 0.2, 0.4); // Orange highlight
                cr->set_line_width(4.0);
                cr->rectangle(rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4);
                cr->stroke();
            }
        }
    }
    
    // Highlight selected circles
    for (int idx : selected_circle_indices) {
        if (idx < completed_circles.size()) {
            for (const auto& circle : completed_circles[idx].circles) {
                cr->set_source_rgba(0.8, 0.4, 0.2, 0.4); // Orange highlight
                cr->set_line_width(4.0);
                cr->arc(circle.x, circle.y, circle.r + 2, 0, 2 * M_PI);
                cr->stroke();
            }
        }
    }
}

bool CairoDrawingArea::is_point_in_stroke(const Stroke& stroke, double x, double y, double tolerance) {
    for (const auto& point : stroke.points) {
        double distance = point_distance(Point(x, y), point);
        if (distance <= tolerance) {
            return true;
        }
    }
    return false;
}

bool CairoDrawingArea::is_point_in_rectangle(const Rect& rect, double x, double y) {
    return x >= rect.x && x <= rect.x + rect.width && 
           y >= rect.y && y <= rect.y + rect.height;
}

bool CairoDrawingArea::is_point_in_circle(const Circle_Data& circle, double x, double y) {
    double distance = point_distance(Point(x, y), Point(circle.x, circle.y));
    return distance <= circle.r;
}

// Stroke class methods
void Stroke::complete_stroke() {
    // Smooth points already calculated in real-time during add_point()
    // Just clear raw points to free memory
    raw_points.clear();
}

// Background surface management (dual-layer architecture like Electron app)
void CairoDrawingArea::initialize_background_surface(int width, int height) {
    // Create background surface to cache completed strokes (like SVG layer)
    background_surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
    background_context = Cairo::Context::create(background_surface);
    
    // Clear to transparent
    background_context->set_source_rgba(0, 0, 0, 0);
    background_context->paint();
    
    background_dirty = false;
}

void CairoDrawingArea::render_stroke_to_background(const Stroke& stroke) {
    if (!background_context || stroke.points.size() < 2) return;
    
    // Render completed stroke to background surface (cached layer)
    background_context->set_source_rgba(stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
    background_context->set_line_width(stroke.width);
    background_context->set_line_cap(Cairo::Context::LineCap::ROUND);
    background_context->set_line_join(Cairo::Context::LineJoin::ROUND);
    
    // Draw the smooth stroke path to background
    background_context->move_to(stroke.points[0].x, stroke.points[0].y);
    for (size_t i = 1; i < stroke.points.size(); i++) {
        background_context->line_to(stroke.points[i].x, stroke.points[i].y);
    }
    background_context->stroke();
}

void CairoDrawingArea::render_rectangle_to_background(const Rectangle& rectangle) {
    if (!background_context) return;
    
    for (const auto& rect : rectangle.rects) {
        background_context->set_source_rgb(rect.color.r, rect.color.g, rect.color.b);
        background_context->set_line_width(2.0);
        background_context->rectangle(rect.x, rect.y, rect.width, rect.height);
        background_context->stroke();
    }
}

void CairoDrawingArea::render_circle_to_background(const Circle& circle) {
    if (!background_context) return;
    
    for (const auto& c : circle.circles) {
        background_context->set_source_rgb(c.color.r, c.color.g, c.color.b);
        background_context->set_line_width(2.0);
        background_context->arc(c.x, c.y, c.r, 0, 2 * M_PI);
        background_context->stroke();
    }
}

void CairoDrawingArea::erase_from_background(double x, double y, double radius) {
    if (!background_context) return;
    
    // Erase from background surface using destination-out blend mode
    background_context->save();
    background_context->set_operator(Cairo::Context::Operator::DEST_OUT);
    background_context->arc(x, y, radius, 0, 2 * M_PI);
    background_context->fill();
    background_context->restore();
}

void CairoDrawingArea::rebuild_background_surface() {
    if (!background_context) return;
    
    // Clear background surface
    background_context->save();
    background_context->set_operator(Cairo::Context::Operator::CLEAR);
    background_context->paint();
    background_context->restore();
    
    // Re-render all remaining strokes
    for (const auto& stroke : completed_strokes) {
        render_stroke_to_background(stroke);
    }
    
    // Re-render rectangles and circles to background too
    for (const auto& rectangle : completed_rectangles) {
        for (const auto& rect : rectangle.rects) {
            background_context->set_source_rgb(rect.color.r, rect.color.g, rect.color.b);
            background_context->set_line_width(2.0);
            background_context->rectangle(rect.x, rect.y, rect.width, rect.height);
            background_context->stroke();
        }
    }
    
    for (const auto& circle : completed_circles) {
        for (const auto& c : circle.circles) {
            background_context->set_source_rgb(c.color.r, c.color.g, c.color.b);
            background_context->set_line_width(2.0);
            background_context->arc(c.x, c.y, c.r, 0, 2 * M_PI);
            background_context->stroke();
        }
    }
}

// Tool change handler function
void on_tool_changed(const std::string& tool_name, CairoDrawingArea* drawing_area) {
    std::cout << "Tool changed to: " << tool_name << std::endl;

    drawing_area->set_drawing_state(tool_name);
    
    // Clear selections when switching tools
    if (tool_name != "select") {
        drawing_area->clear_selection();
    }
}
