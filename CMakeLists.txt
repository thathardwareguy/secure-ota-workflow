cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Use execute_process command to get version information from Git
execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Check if Git command was successful
if(NOT GIT_VERSION)
    # Set default values if Git is not available or command failed
    set(BUILD_TAG "default")
    set(BUILD_INFO "")
    message("Git command failed: ${GIT_ERROR}")
    return()
endif()

string(REPLACE "-" " " VERSION_LIST ${GIT_VERSION})
list(GET VERSION_LIST 0 BUILD_TAG)

# Print the version information
message("Build tag: ${BUILD_TAG}")

# Rest of your CMake configuration

set(PROJECT_VER ${BUILD_TAG})
string(REPLACE "." "_" FILE_VER ${BUILD_TAG})
set(FILE_NAME "firmware_esp32_${FILE_VER}")
project(${FILE_NAME})