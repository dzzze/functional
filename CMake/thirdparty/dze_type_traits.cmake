include_guard(GLOBAL)

include(fetch_content)
include(thirdparty_common)

set(proj_name dze_type_traits)

fetch_content(
    ${proj_name}_proj
    GIT_REPOSITORY https://github.com/dzzze/type_traits.git
    GIT_TAG f002dab81e529f74dc872dd588ca6597ac1ac41b
    GIT_SHALLOW true
    PREFIX "${thirdparty_prefix}/${proj_name}"
    SOURCE_DIR "${thirdparty_prefix}/${proj_name}/source"
    BINARY_DIR "${thirdparty_binary_dir}/${proj_name}/bin")

add_subdirectory(${${proj_name}_proj_SOURCE_DIR} ${${proj_name}_proj_BINARY_DIR} EXCLUDE_FROM_ALL)

unset(proj_name)
