cmake_minimum_required(VERSION 3.0)

project(stream_viz)

set(CMAKE_CXX_STANDARD 14)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)

add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Werror=return-type)

include_directories("/usr/include/hdf5/serial")
link_directories("/usr/lib/x86_64-linux-gnu/hdf5/serial")

include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(project)
add_subdirectory(app)
add_subdirectory(test)
