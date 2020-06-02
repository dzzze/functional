include_guard(GLOBAL)

include(FetchContent)

set(proj_name dze_type_traits)

FetchContent_Populate(
    ${proj_name}_proj
    GIT_REPOSITORY https://github.com/dzzze/type_traits.git
    GIT_TAG 76318a79be1be64e00730e3413ac2c36a242a885
    GIT_SHALLOW true
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}"
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}/source"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${proj_name}/bin")

add_subdirectory(${${proj_name}_proj_SOURCE_DIR} ${${proj_name}_proj_BINARY_DIR} EXCLUDE_FROM_ALL)

unset(proj_name)
