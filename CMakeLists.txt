cmake_minimum_required(VERSION 3.0)

project(stream_viz)

set(CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)

include_directories("/usr/include/hdf5/serial")
link_directories("/usr/lib/x86_64-linux-gnu/hdf5/serial")

add_subdirectory(test)
