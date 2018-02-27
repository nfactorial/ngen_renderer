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
        source/vertex_array_object.cpp
        source/shader_program.cpp
        source/buffer_object.cpp
        source/shader_object.cpp
        source/context_state.cpp
        source/texture.cpp)

set(INCLUDE_FILES
        include/GL/glew.h
        include/GL/eglew.h
        include/GL/glxew.h
        include/GL/wglew.h
        include/platform.h
        include/context_state.h
        include/vertex_array_object.h
        include/shader_program.h
        include/buffer_object.h
        include/shader_object.h
        include/texture.h)

add_library(ngen_opengl STATIC
    ${SOURCE_FILES}
    ${INCLUDE_FILES}
)

add_library(ngen::ogl ALIAS ngen_opengl)

target_include_directories(ngen_opengl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/ngen/renderer/opengl>
)

install(TARGETS ngen_opengl EXPORT ngen_opengl_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(EXPORT ngen_opengl_targets
    FILE ngen_opengl_targets.cmake
    NAMESPACE ngen::
    DESTINATION lib/cmake/ngen
)

if (NGEN_BUILD_TESTS)
    add_subdirectory(test)
endif()
