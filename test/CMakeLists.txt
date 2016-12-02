
add_executable(test-array test_array.cpp)

add_executable(test-hdf5 test_hdf5.cpp)
target_link_libraries(test-hdf5 -lhdf5_cpp -lhdf5)

set(plot_src
  ../plot/line_plot.cpp
  ../plot/heat_map.cpp
  ../plot/plot_view.cpp
)

add_executable(test-plot test_plot.cpp ${plot_src})
target_link_libraries(test-plot Qt5::Widgets)

add_executable(test-plot-hdf5 test_plot_hdf5.cpp ${plot_src})
target_link_libraries(test-plot-hdf5 Qt5::Widgets -lhdf5_cpp -lhdf5)
