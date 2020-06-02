include_guard(GLOBAL)

include(FetchContent)

set(proj_name dze_type_traits_proj)

FetchContent_Populate(
    ${proj_name}
    GIT_REPOSITORY https://github.com/dzzze/type_traits.git
    GIT_TAG 76318a79be1be64e00730e3413ac2c36a242a885
    GIT_SHALLOW true
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/Catch2"
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/Catch2/source"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Catch2/bin")

add_subdirectory(${${proj_name}_SOURCE_DIR} ${${proj_name}_BINARY_DIR} EXCLUDE_FROM_ALL)
