include_guard(GLOBAL)

include(FetchContent)

FetchContent_Populate(
    add_custom_test_proj
    GIT_REPOSITORY https://github.com/dzzze/add_custom_test
    GIT_TAG 52b73546033586bd5b2495d45a89799274dfe78f
    GIT_SHALLOW true
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/add_custom_test"
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/add_custom_test/source")

include(${add_custom_test_proj_SOURCE_DIR}/add_custom_test.cmake)
