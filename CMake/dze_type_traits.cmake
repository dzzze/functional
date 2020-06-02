include_guard(GLOBAL)

include(fetch_content)
include(thirdparty_common)

set(proj_name dze_type_traits)

fetch_content(
    ${proj_name}_proj
    GIT_REPOSITORY https://github.com/dzzze/type_traits.git
    GIT_TAG 76318a79be1be64e00730e3413ac2c36a242a885
    GIT_SHALLOW true
    PREFIX "${thirdparty_prefix}/${proj_name}"
    SOURCE_DIR "${thirdparty_prefix}/${proj_name}/source"
    BINARY_DIR "${thirdparty_binary_dir}/${proj_name}/bin")

add_subdirectory(${${proj_name}_proj_SOURCE_DIR} ${${proj_name}_proj_BINARY_DIR} EXCLUDE_FROM_ALL)

unset(proj_name)
