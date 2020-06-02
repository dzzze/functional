include_guard(GLOBAL)

include(FetchContent)

set(proj_name add_custom_test_proj)

FetchContent_Populate(
    ${proj_name}
    GIT_REPOSITORY https://github.com/dzzze/add_custom_test
    GIT_TAG 52b73546033586bd5b2495d45a89799274dfe78f
    GIT_SHALLOW true
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/add_custom_test"
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/add_custom_test/source")

include(${${proj_name}_SOURCE_DIR}/add_custom_test.cmake)
