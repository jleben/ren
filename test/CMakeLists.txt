
add_executable(test-array test_array.cpp)

set(test_plot_src
  test_plot.cpp
  ../plot/line_plot.cpp
  ../plot/plot_view.cpp
)

add_executable(test-plot ${test_plot_src})
target_link_libraries(test-plot Qt5::Widgets)
