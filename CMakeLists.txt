cmake_minimum_required(VERSION 3.13)

project(dze_optional)

option(${PROJECT_NAME}_unit_tests "Enable unit tests" ON)
option(${PROJECT_NAME}_sanitize_address "Enable address sanitizer" OFF)
option(${PROJECT_NAME}_sanitize_ub "Enable undefined behaviour sanitizer" OFF)
option(${PROJECT_NAME}_static_analyzer "Enable Clang-Tidy" ON)

set(hikari_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(hikari_binary_dir ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${hikari_source_dir}/CMake")

include(compiler_options) # Project wide options that also apply to thirdparty projects.
#include(thirdparty_common)

add_library(dze_optional INTERFACE)
target_include_directories(dze_optional INTERFACE include)
add_library(dze::optional ALIAS dze_optional)

if (${PROJECT_NAME}_static_analyzer)
    include(clang-tidy)
endif ()

if (${PROJECT_NAME}_unit_tests)
    enable_testing()
    add_custom_target(${PROJECT_NAME}-run-custom-tests ALL)
    add_subdirectory(test)
endif ()
