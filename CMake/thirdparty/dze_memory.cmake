include_guard(GLOBAL)

include(fetch_content)
include(thirdparty_common)

set(proj_name dze_memory)

fetch_content(
    ${proj_name}_proj
    GIT_REPOSITORY https://github.com/dzzze/memory.git
    GIT_TAG 06fc8cb8e3161d88b7cf2b179c7d0d92bd8c8aff
    GIT_SHALLOW true
    PREFIX "${thirdparty_prefix}/${proj_name}"
    SOURCE_DIR "${thirdparty_prefix}/${proj_name}/source"
    BINARY_DIR "${thirdparty_binary_dir}/${proj_name}/bin")

add_subdirectory(${${proj_name}_proj_SOURCE_DIR} ${${proj_name}_proj_BINARY_DIR} EXCLUDE_FROM_ALL)

unset(proj_name)
