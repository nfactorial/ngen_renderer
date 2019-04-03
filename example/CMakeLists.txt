project(ngen_render_example)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES x86_64)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# add_definitions(-DSDL_MAIN_NEEDED)
add_definitions(-DSDL_MAIN_HANDLED)
add_definitions(-DSDL_STATIC)

MESSAGE( "SDL2 LIBRARY = ${SDL2_LIBRARY}")
MESSAGE( "SDL2_INCLUDE_DIR = " ${SDL2_INCLUDE_DIR})

add_executable(example
        main.cpp
        )

target_include_directories(example PUBLIC
        $<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>
        )

target_compile_definitions(example PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_include_directories(example PRIVATE Vulkan::Vulkan)

target_link_libraries(example Vulkan::Vulkan)
target_link_libraries(example SDL2::Main)
target_link_libraries(example ngen::vulkan)
