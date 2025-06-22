# GTK4 Drawing Application

A feature-rich drawing application built with GTK4 and C++ that provides multiple drawing tools including pen, shapes, eraser, and selection functionality.

## Features

### Drawing Tools
- **Pen Tool**: Ultra-smooth stroke rendering with Catmull-Rom spline interpolation
- **Rectangle Tool**: Click and drag to create rectangles with real-time preview
- **Circle Tool**: Click and drag to create circles with real-time preview
- **Eraser Tool**: Collision detection with visual preview for object removal
- **Selection Tool**: Multi-object selection with drag-to-select and move functionality

### Advanced Features
- Professional-quality smooth stroke rendering
- Multi-object selection and manipulation
- Real-time visual feedback and previews
- Performance-optimized rendering pipeline

## Architecture

### Core Components
- **Point**: 2D coordinates with timestamp for stroke tracking
- **Color**: RGB color system with defaults
- **Stroke**: Pen strokes with smooth Catmull-Rom interpolation
- **Rect/Circle_Data**: Shape primitives with color support

### Drawing System
The application uses a dual-system approach:
- **Legacy System**: Direct vector storage for backward compatibility
- **Object System**: Modern object-oriented approach with selection support

## Usage

### Drawing
1. Select the pen tool and draw smooth strokes on the canvas
2. Use rectangle/circle tools by clicking and dragging
3. Use the eraser tool to remove objects by dragging over them

### Selection and Moving
1. Select the selection tool from the toolbar
2. **Select objects**: Click and drag to draw a blue selection rectangle
3. **Move objects**: Click on highlighted (orange) selected objects and drag to move
4. **Clear selection**: Switch tools or click in empty space

## Building

### Prerequisites
- GTK4 development libraries
- Cairo graphics library
- CMake build system
- C++ compiler with C++11 support

### Build Instructions
```bash
mkdir build
cd build
cmake ..
make
```

### Running
```bash
./drawing-app
```

## Technical Details

### Smooth Stroke Rendering
- Point simplification to reduce noise
- Ultra-smooth stroke interpolation using Catmull-Rom splines
- 12 segments per curve for professional quality
- Density control for performance optimization

### Selection System
- Index-based tracking for all object types
- Visual feedback with orange highlights
- Drag-to-select rectangle in blue
- Multi-object movement support

### Color System
- Default stroke color: Blue (0.0, 0.0, 0.8)
- Default shapes: Black (0.0, 0.0, 0.0)
- Selection highlights: Orange (0.8, 0.4, 0.2)
- Selection rectangle: Blue (0.2, 0.4, 0.8)

## Code Structure
- `src/drawingLogic.hpp` - Class definitions and declarations
- `src/drawingLogic.cpp` - Implementation and rendering logic
- `src/main.cpp` - Application entry point
- `CMakeLists.txt` - Build configuration

## Future Enhancements
- Resize functionality for selected objects
- Copy/paste operations
- Layer system
- Undo/redo functionality
- Multi-select with keyboard modifiers
- Group/ungroup operations

## License
This project is open source. Feel free to use and modify as needed.