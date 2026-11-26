# Runs the example from a folder with no shaders/triangle.spv and checks it names the file on stderr,
# prints nothing to stdout and exits with status 1.
file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

execute_process(
    COMMAND "${EXAMPLE}"
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE exit_code
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
)

if (NOT exit_code EQUAL 1)
    message(FATAL_ERROR "Expected exit status 1, got '${exit_code}'.")
endif()
if (NOT err MATCHES "shaders/triangle\.spv")
    message(FATAL_ERROR "stderr does not name shaders/triangle.spv: '${err}'")
endif()
if (NOT out STREQUAL "")
    message(FATAL_ERROR "Expected nothing on stdout, got: '${out}'")
endif()
