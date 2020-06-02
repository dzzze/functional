include_guard(GLOBAL)

include(FetchContent)

set(proj_name catch2_proj)

FetchContent_Populate(
    ${proj_name}
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG b1b5cb812277f367387844aab46eb2d3b15d03cd
    GIT_SHALLOW true
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/Catch2"
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/Catch2/source"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Catch2/bin")

set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
add_subdirectory(${${proj_name}_SOURCE_DIR} ${${proj_name}_BINARY_DIR} EXCLUDE_FROM_ALL)

add_library(Catch2Main OBJECT ${CMAKE_CURRENT_LIST_DIR}/catch_with_main.cpp)
target_link_libraries(Catch2Main Catch2)
add_library(Catch2::Main ALIAS Catch2Main)
