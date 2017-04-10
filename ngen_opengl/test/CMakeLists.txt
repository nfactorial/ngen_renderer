project(ngen_opengl_tests)

find_package(SDL2 REQUIRED)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${SDL2_INCLUDE_DIR})

include_directories(${SDL_ROOT_DIRECTORY}/include)

# add_definitions(-DSDL_MAIN_NEEDED)
add_definitions(-DSDL_MAIN_HANDLED)

# set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
# set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

# link_directories(${SDL_ROOT_DIRECTORY}/lib/x64)

MESSAGE( "SDL2 LIBRARY = ${SDL2_LIBRARY}")
# MESSAGE( "SDL2_INCLUDE_DIR = " ${SDL2_INCLUDE_DIR})

add_executable(ngen_opengl_tests
        main.cpp
        test_shader_program.cpp
        test_buffer_object.cpp
        test_shader_object.cpp
        test_texture.cpp)

# target_link_libraries(ngen_opengl_tests SDL2 SDL2main)
target_link_libraries(ngen_opengl_tests ${SDL2_LIBRARY})
target_link_libraries(ngen_opengl_tests ngen_opengl opengl32)
target_link_libraries(ngen_opengl_tests gtest)
