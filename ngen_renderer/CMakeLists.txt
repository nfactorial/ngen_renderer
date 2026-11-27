add_library(ngen_renderer STATIC
    source/configuration.cpp
    source/render_graph.cpp
    source/renderer.cpp
)
add_library(ngen::renderer ALIAS ngen_renderer)

target_include_directories(ngen_renderer PUBLIC include)
target_link_libraries(ngen_renderer PUBLIC ngen::rhi)

if (NGEN_BUILD_TESTS)
    add_subdirectory(tests)
endif()
