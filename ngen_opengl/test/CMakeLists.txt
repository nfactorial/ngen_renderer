project(ngen_opengl_tests)

# TODO: This shouldn't be hard-coded in here
set(SDL2_PATH "C:\\Library\\Frameworks\\SDL2-2.0.8\\i686-w64-mingw32")

find_package(SDL2 REQUIRED)

# add_definitions(-DSDL_MAIN_NEEDED)
add_definitions(-DSDL_MAIN_HANDLED)

# MESSAGE( "SDL2 LIBRARY = ${SDL2_LIBRARY}")
# MESSAGE( "SDL2_INCLUDE_DIR = " ${SDL2_INCLUDE_DIR})

add_executable(ngen_opengl_tests
        main.cpp
        test_shader_program.cpp
        test_buffer_object.cpp
        test_shader_object.cpp
        test_texture.cpp)

target_include_directories(ngen_opengl_tests PUBLIC
    $<BUILD_INTERFACE:${gtest_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${gtest_SOURCE_DIR}>
    $<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>
)

target_link_libraries(ngen_opengl_tests ${SDL2_LIBRARY})
target_link_libraries(ngen_opengl_tests ngen::ogl opengl32)
target_link_libraries(ngen_opengl_tests gtest)
