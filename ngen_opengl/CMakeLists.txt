cmake_minimum_required(VERSION 3.1)

project(ngen_opengl)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES x86_64)

# add_definitions(-DGLEW_STATIC -D_WIN32)
add_definitions(-DGLEW_STATIC)

set(SOURCE_FILES
        source/glew.cpp
        source/shader_program.cpp
        source/buffer_object.cpp
        source/shader_object.cpp
        source/texture.cpp)

set(INCLUDE_FILES
        include/GL/glew.h
        include/GL/eglew.h
        include/GL/glxew.h
        include/GL/wglew.h
        include/platform.h
        include/shader_program.h
        include/buffer_object.h
        include/shader_object.h
        include/texture.h)

include_directories(include)
include_directories(${GLEW_ROOT_DIRECTORY}/include)

add_library(ngen_opengl STATIC ${SOURCE_FILES} ${INCLUDE_FILES})

if (NGEN_BUILD_TESTS)
    add_subdirectory(test)
endif()
