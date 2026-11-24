add_library(ngen_directx12 STATIC
    source/context.cpp
)
add_library(ngen::directx12 ALIAS ngen_directx12)

target_include_directories(ngen_directx12 PUBLIC include)
target_compile_definitions(ngen_directx12 PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
target_link_libraries(ngen_directx12 PUBLIC d3d12 dxgi)
