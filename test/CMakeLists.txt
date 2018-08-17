
add_executable(run_tests
    test.cpp
    test_text_source.cpp
    ../testing/testing.cpp
    ../data/data_set.cpp
    ../data/dimension.cpp
    ../io/text.cpp
)

target_link_libraries(run_tests ${datavis_linked_libs})
