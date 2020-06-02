include_guard(GLOBAL)

include(FetchContent)

set(proj_name add_custom_test)

FetchContent_Populate(
    ${proj_name}_proj
    GIT_REPOSITORY https://github.com/dzzze/add_custom_test
    GIT_TAG 52b73546033586bd5b2495d45a89799274dfe78f
    GIT_SHALLOW true
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}"
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}/source")

include(${${proj_name}_proj_SOURCE_DIR}/add_custom_test.cmake)

unset(proj_name)
