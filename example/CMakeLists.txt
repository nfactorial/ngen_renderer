add_executable(example
    main.cpp
    render.cpp
)

target_link_libraries(example PRIVATE ngen::vulkan)

# main.cpp provides its own main() rather than linking SDL2main.
target_compile_definitions(example PRIVATE SDL_MAIN_HANDLED)

# Compile the GLSL shaders to SPIR-V next to the executable, where render.cpp loads them from
# (./shaders/vert.spv and ./shaders/frag.spv, relative to the working directory).
if (NOT Vulkan_GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc was not found; it ships with the Vulkan SDK.")
endif()

set(SHADER_OUTPUTS)
foreach (stage IN ITEMS vert frag)
    set(source ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.${stage}.glsl)
    set(output ${CMAKE_CURRENT_BINARY_DIR}/shaders/${stage}.spv)

    add_custom_command(
        OUTPUT  ${output}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} -fshader-stage=${stage} ${source} -o ${output}
        DEPENDS ${source}
        COMMENT "Compiling shader.${stage}.glsl"
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

# A shared SDL2 needs its DLL next to the executable on Windows.
if (WIN32)
    add_custom_command(TARGET example POST_BUILD
        # Skipped when there are no DLLs (e.g. SDL2 built statically).
        COMMAND "$<$<BOOL:$<TARGET_RUNTIME_DLLS:example>>:${CMAKE_COMMAND};-E;copy_if_different;$<TARGET_RUNTIME_DLLS:example>;$<TARGET_FILE_DIR:example>>"
        COMMAND_EXPAND_LISTS
    )
endif()

# Run from the executable's folder in Visual Studio so the relative shader paths resolve.
set_target_properties(example PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:example>)
