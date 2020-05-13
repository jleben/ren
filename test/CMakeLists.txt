
add_executable(run_tests
    test.cpp
    test_text_source.cpp
    ../reactive/test_reactive.cpp
    ../testing/testing.cpp
)

target_link_libraries(run_tests ren_core ${ren_linked_libs})
