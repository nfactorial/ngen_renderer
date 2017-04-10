cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES x86_64)

add_subdirectory(external)

option(NGEN_BUILD_TESTS "Build unit tests." ON)

project(ngen_renderer)

include_directories(SYSTEM external/ngen/include)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

# Enable correct rendering API
if (NGEN_BUILD_OPENGL)
    find_package(OpenGL REQUIRED)

    if(OPENGL_FOUND)
        MESSAGE( "OpenGL library found successfully.")
        include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})
    else(OPENGL_FOUND)
        MESSAGE( "Unable to find OpenGL library, please make sure it is installed." )
    endif (OPENGL_FOUND)

    include_directories(${ngenOpenGL_SOURCE_DIR}/include)
endif()

add_subdirectory(ngen_opengl)
