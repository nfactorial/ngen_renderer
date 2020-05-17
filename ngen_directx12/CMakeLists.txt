cmake_minimum_required(VERSION 3.1)

project(ngen_directx12)

message(STATUS "We are in the dx12 lib")

set(SOURCE_FILES
        source/context.cpp
        )

set(INCLUDE_FILES
        include/com_ptr.h
        include/context.h
        )

add_definitions(-DNOMINMAX)

add_library(ngen_directx12 STATIC
    ${SOURCE_FILES}
    ${INCLUDE_FILES}
)

add_library(ngen::directx12 ALIAS ngen_directx12)

target_include_directories(ngen_directx12 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/ngen/renderer/directx12>
)

install(TARGETS ngen_directx12 EXPORT ngen_directx12_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(EXPORT ngen_directx12_targets
    FILE ngen_directx12_targets.cmake
    NAMESPACE ngen::
    DESTINATION lib/cmake/ngen
)

if (NGEN_BUILD_TESTS)
    # add_subdirectory(test)
endif()
