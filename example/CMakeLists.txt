project(ngen_opengl_tests)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES x86_64)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# TODO: This shouldn't be hard-coded in here
set(SDL2_PATH "C:\\Library\\Frameworks\\SDL2-2.0.8\\i686-w64-mingw32")

find_package(SDL2 REQUIRED)

# add_definitions(-DSDL_MAIN_NEEDED)
add_definitions(-DSDL_MAIN_HANDLED)
add_definitions(-DGLEW_STATIC)
add_definitions(-DSDL_STATIC)

# MESSAGE( "SDL2 LIBRARY = ${SDL2_LIBRARY}")
# MESSAGE( "SDL2_INCLUDE_DIR = " ${SDL2_INCLUDE_DIR})

add_executable(example
        main.cpp
        )

target_include_directories(example PUBLIC
        $<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>
        )

target_link_libraries(example ${SDL2_LIBRARY})
target_link_libraries(example ngen::ogl opengl32)
