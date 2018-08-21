
set(plot_src
  ../plot/selector.hpp
  ../plot/plot.hpp
  ../plot/line_plot.cpp
  ../plot/heat_map.cpp
  ../plot/scatter_plot_1d.cpp
  ../plot/scatter_plot_2d.cpp
  ../plot/plot_view.cpp
  ../plot/range_bar.cpp
)

set(app_src
  main.cpp
  data_library_view.cpp
  data_info_view.cpp
  color_box.cpp
  settings_view.cpp
  plot_data_settings_view.cpp
  plot_settings_view2.cpp
  main_window.cpp
  ../io/hdf5.cpp
  ../io/text.cpp
  ../io/sndfile.cpp
  ../data/data_set.cpp
  ../data/data_library.cpp
  ../data/dimension.cpp
  ${plot_src}
)

add_executable(datavis ${app_src})

target_link_libraries(datavis ${datavis_linked_libs})

