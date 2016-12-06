
set(plot_src
  ../plot/plot.hpp
  ../plot/line_plot.cpp
  ../plot/heat_map.cpp
  ../plot/plot_view.cpp
)

set(app_src
  main.cpp
  color_box.cpp
  data_source.cpp
  settings_view.cpp
  line_plot_settings_view.cpp
  main_window.cpp
  ${plot_src}
)

add_executable(datavis ${app_src})

target_link_libraries(datavis Qt5::Widgets hdf5_cpp hdf5)
