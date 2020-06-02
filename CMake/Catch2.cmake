include_guard(GLOBAL)

include(fetch_content)

set(proj_name catch2)

fetch_content(
    ${proj_name}_proj
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG b1b5cb812277f367387844aab46eb2d3b15d03cd
    GIT_SHALLOW true
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}"
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}/source"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}/bin")

add_subdirectory(${${proj_name}_proj_SOURCE_DIR} ${${proj_name}_proj_BINARY_DIR} EXCLUDE_FROM_ALL)

unset(proj_name)

add_library(Catch2Main OBJECT ${CMAKE_CURRENT_LIST_DIR}/catch_with_main.cpp)
target_link_libraries(Catch2Main Catch2)
add_library(Catch2::Main ALIAS Catch2Main)
