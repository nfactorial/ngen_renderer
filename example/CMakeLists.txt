add_executable(example
    main.cpp
    render.cpp
)

target_link_libraries(example PRIVATE ngen::platform ngen::renderer ngen::rhi_vulkan ${NGEN_SDL3_TARGET})

# main.cpp provides its own main() rather than using SDL's main redirection.
target_compile_definitions(example PRIVATE SDL_MAIN_HANDLED)

# Compile the Slang shaders to SPIR-V next to the executable, where render.cpp loads them from
# (shaders/triangle.spv, relative to the working directory). slangc ships with the Vulkan SDK.
find_program(SLANGC_EXECUTABLE slangc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" REQUIRED)

set(SHADER_OUTPUTS)
foreach (shader IN ITEMS triangle)
    set(source ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shader}.slang)
    set(output ${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader}.spv)

    # Every module provides vertexMain and fragmentMain; the entry point names are kept in the SPIR-V.
    # SPIR-V 1.3 matches the Vulkan 1.1 instance version the Vulkan backend requests.
    add_custom_command(
        OUTPUT  ${output}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMAND ${SLANGC_EXECUTABLE} ${source} -target spirv -profile spirv_1_3 -fvk-use-entrypoint-name
                -entry vertexMain -stage vertex -entry fragmentMain -stage fragment -o ${output}
        DEPENDS ${source}
        COMMENT "Compiling ${shader}.slang"
        VERBATIM
    )
    list(APPEND SHADER_OUTPUTS ${output})
endforeach()

add_custom_target(example_shaders DEPENDS ${SHADER_OUTPUTS})
add_dependencies(example example_shaders)

add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders $<TARGET_FILE_DIR:example>/shaders
    VERBATIM
)

# A shared SDL3 needs its DLL next to the executable on Windows.
if (WIN32)
    add_custom_command(TARGET example POST_BUILD
        # Skipped when there are no DLLs (e.g. SDL3 built statically).
        COMMAND "$<$<BOOL:$<TARGET_RUNTIME_DLLS:example>>:${CMAKE_COMMAND};-E;copy_if_different;$<TARGET_RUNTIME_DLLS:example>;$<TARGET_FILE_DIR:example>>"
        COMMAND_EXPAND_LISTS
    )
endif()

# Run from the executable's folder in Visual Studio so the relative shader paths resolve.
set_target_properties(example PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:example>)

# With the compiled shader missing, the example reports it and exits with status 1, printing nothing to stdout.
if (NGEN_BUILD_TESTS)
    add_test(NAME example_missing_shader
        COMMAND ${CMAKE_COMMAND}
            -DEXAMPLE=$<TARGET_FILE:example>
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/missing_shader_cwd
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/missing_shader.cmake
    )
endif()
