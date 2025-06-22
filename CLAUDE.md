# GTK Drawing Application - Project Documentation

## Project Overview
This is a GTK4 drawing application written in C++ that provides a canvas with multiple drawing tools including pen, rectangle, circle, eraser, and selection tools.

## Architecture

### Core Classes

#### `Point` (lines 11-16 in drawingLogic.hpp)
- Represents a 2D point with x, y coordinates and timestamp
- Used for stroke points and mouse positions

#### `Color` (lines 18-21)
- RGB color representation with default constructor
- Used throughout the application for drawing colors

#### `Stroke` (lines 80-90)
- Represents a pen stroke with points, color, and width
- **IMPORTANT**: This class was moved to come before `StrokeObject` in the header file to fix dependency order
- Contains vector of points and drawing properties
- Supports smooth rendering with Catmull-Rom interpolation

#### `Rect` (lines 23-29)
- Rectangle structure with position, dimensions, and color

#### `Circle_Data` (lines 31-37)
- Circle structure with center, radius, and color

### Drawing System

#### Legacy Drawing System (preserved)
The application maintains the original drawing system for backward compatibility:
- `completed_strokes` - vector of completed pen strokes
- `completed_rectangles` - vector of completed rectangles
- `completed_circles` - vector of completed circles

#### Unified Object System (partial implementation)
A newer object-oriented system exists alongside the legacy system:
- `DrawableObject` base class with selection and transformation capabilities
- `StrokeObject`, `RectangleObject`, `CircleObject` concrete implementations
- `SelectionManager` for handling multiple selections

### Tools Implementation

#### Pen Tool
- Creates smooth strokes using advanced interpolation
- **Key Features**: Ultra-smooth stroke rendering with Catmull-Rom splines
- Point simplification to reduce noise
- Density control for performance optimization

#### Rectangle Tool
- Click and drag to create rectangles
- Real-time preview during drawing

#### Circle Tool
- Click and drag to create circles (radius determined by distance)
- Real-time preview during drawing

#### Eraser Tool
- **WORKING**: Collision detection with all object types
- Visual preview showing objects to be erased
- Removes objects on contact during drag

#### Selection Tool (newly implemented)
**COMPLETE IMPLEMENTATION WITHOUT BREAKING EXISTING FUNCTIONALITY**

##### Features:
1. **Drag Selection**: Draw a blue rectangle to select multiple objects
2. **Visual Feedback**: Orange highlights show selected objects
3. **Move Objects**: Click and drag selected objects to move them
4. **Multi-object Support**: Works with strokes, rectangles, and circles
5. **Smart Clearing**: Selections clear when switching tools

##### Implementation Details:
- **Selection State Variables**:
  - `selected_stroke_indices` - indices of selected strokes
  - `selected_rectangle_indices` - indices of selected rectangles  
  - `selected_circle_indices` - indices of selected circles
  - `is_selecting` - drawing selection rectangle
  - `is_moving_selection` - moving selected objects

- **Key Methods**:
  - `clear_all_selections()` - private method to clear all selections
  - `clear_selection()` - public method with redraw
  - `select_objects_in_rectangle()` - select objects within bounds
  - `move_selected_objects()` - move all selected objects by delta
  - `draw_selection_rectangle()` - draw blue selection rectangle
  - `draw_selection_highlights()` - draw orange object highlights
  - Collision detection methods for each object type

##### Input Handling:
- **Mouse Press**: Start selection rectangle OR start moving if clicking on selected object
- **Mouse Motion**: Update selection rectangle OR move selected objects
- **Mouse Release**: Complete selection OR finish moving

### Rendering Pipeline

#### on_draw() Method (lines 189-280 in drawingLogic.cpp)
**CAREFULLY PRESERVED - DOES NOT BREAK EXISTING LOGIC**

1. Clear background (transparent)
2. Draw completed strokes (with smooth rendering)
3. Draw completed rectangles
4. Draw completed circles
5. **NEW**: Draw selection highlights for selected objects
6. **NEW**: Draw selection rectangle if actively selecting
7. Draw current stroke/rectangle/circle previews
8. Draw eraser previews

### Smooth Stroke Rendering

#### Advanced Interpolation System
- `simplify_stroke()` - removes micro-jitter with tolerance
- `ultra_smooth_stroke()` - applies heavy smoothing
- `interpolate_catmull_rom()` - creates ultra-smooth curves with 12 segments per curve
- `catmull_rom_point()` - calculates interpolated points using Catmull-Rom spline formula

This creates professional-quality smooth strokes that eliminate jaggedness.

### Input Handling

#### Tool-Specific Event Handling
- **Pen**: Dense point collection (0.5px threshold) for smooth strokes
- **Rectangle/Circle**: Preview updates on motion
- **Eraser**: Real-time collision detection and preview
- **Selection**: Rectangle selection and object moving

### Color System
- Default stroke color: blue (0.0, 0.0, 0.8)
- Default rectangle/circle color: black (0.0, 0.0, 0.0)
- Selection highlights: orange (0.8, 0.4, 0.2)
- Selection rectangle: blue (0.2, 0.4, 0.8)

## Important Implementation Notes

### What Was Preserved
- **All original smooth stroke rendering logic**
- **All eraser functionality** 
- **All drawing tool functionality**
- **Original object vectors and rendering**
- **Performance optimizations**

### What Was Added (Selection Tool)
- Index-based selection tracking system
- Visual feedback for selections
- Drag-to-select rectangle
- Move selected objects functionality
- Integration with existing input handling
- Tool switching with automatic selection clearing

### Code Organization
- Header file: Class definitions and method declarations
- Implementation file: All method implementations
- Selection functions added at end of file (lines 1075+)
- Tool change handler updated to clear selections

### Build System
- CMake build system
- GTK4 and Cairo dependencies
- Located in `build/` directory

## Usage Instructions

### Drawing
1. Select pen tool and draw smooth strokes
2. Select rectangle/circle tools and drag to create shapes
3. Select eraser tool and drag over objects to remove them

### Selection and Moving
1. Select the selection tool
2. **To select objects**: Click and drag to draw a selection rectangle
3. **To move objects**: Click on highlighted (selected) objects and drag
4. **To clear selection**: Switch tools or click in empty space

## Future Enhancements Possible
- Resize functionality for selected objects
- Copy/paste for selected objects
- Layer system integration
- Undo/redo for selection operations
- Multi-select with Ctrl key
- Group/ungroup functionality

## Critical Code Locations
- Main drawing logic: `src/drawingLogic.cpp`
- Class definitions: `src/drawingLogic.hpp`
- Smooth stroke rendering: lines 254-362 in drawingLogic.cpp
- Selection system: lines 1075+ in drawingLogic.cpp
- Input handling: lines 48-220 in drawingLogic.cpp