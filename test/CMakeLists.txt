
add_executable(run_tests
    test.cpp
    test_text_source.cpp
    ../reactive/test_reactive.cpp
    ../testing/testing.cpp
)

target_link_libraries(run_tests datavis_core ${datavis_linked_libs})
