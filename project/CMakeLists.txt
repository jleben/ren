 
add_custom_command(
  OUTPUT project.pb.h project.pb.cc
  DEPENDS project.proto
  COMMAND protoc
    -I${CMAKE_CURRENT_SOURCE_DIR}
    --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/project.proto
)

add_library(datavis_project STATIC project.pb.cc)
