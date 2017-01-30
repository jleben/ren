 
add_custom_command(
  OUTPUT project.pb.h
  DEPENDS project.proto
  COMMAND protoc
    -I${CMAKE_CURRENT_SOURCE_DIR}
    --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/project.proto
)

add_custom_target(project_protobuf DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/project.pb.h)
