
# Library

set(plot_src
  ../plot/selector.hpp
  ../plot/plot.hpp
  ../plot/line_plot.cpp
  ../plot/heat_map.cpp
  ../plot/plot_view.cpp
  ../plot/range_bar.cpp
  ../io/hdf5.cpp
  ../data/data_set.cpp
)


add_library(plottestlib STATIC ${plot_src})
target_link_libraries(plottestlib Qt5::Widgets -lhdf5_cpp -lhdf5)

## Executables

add_executable(test-array test_array.cpp)

add_executable(test-hdf5 test_hdf5.cpp)
target_link_libraries(test-hdf5 plottestlib)

add_executable(test-line-plot test_line_plot.cpp)
target_link_libraries(test-line-plot plottestlib)

add_executable(test-heatmap-plot test_heatmap_plot.cpp)
target_link_libraries(test-heatmap-plot plottestlib)

add_executable(test-multi-plots test_multi_plots.cpp)
target_link_libraries(test-multi-plots plottestlib)

add_executable(test-plot-hdf5 test_plot_hdf5.cpp)
target_link_libraries(test-plot-hdf5 plottestlib)
