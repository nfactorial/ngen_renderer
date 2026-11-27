add_executable(ngen_renderer_tests
    render_graph_tests.cpp
    renderer_tests.cpp
)

target_link_libraries(ngen_renderer_tests PRIVATE
    ngen::renderer
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ngen_renderer_tests)
