cmake_minimum_required(VERSION 3.16)
project(gtk-project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtkmm-4.0)
pkg_check_modules(EPOXY REQUIRED epoxy)

add_executable(main 
               src/main.cpp
               src/ui.cpp
               src/canvas.cpp
               src/drawingLogic.cpp
)

target_include_directories(main PRIVATE ${GTK4_INCLUDE_DIRS} ${EPOXY_INCLUDE_DIRS})
target_link_directories(main PRIVATE ${GTK4_LIBRARY_DIRS})
target_compile_options(main PRIVATE ${GTK4_CFLAGS_OTHER})

# Use PRIVATE keyword consistently for all libraries
target_link_libraries(main PRIVATE 
    ${GTK4_LIBRARIES} 
    ${EPOXY_LIBRARIES}
)
