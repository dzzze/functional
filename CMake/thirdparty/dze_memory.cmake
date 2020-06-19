include_guard(GLOBAL)

include(fetch_content)
include(thirdparty_common)

set(proj_name dze_memory)

fetch_content(
    ${proj_name}_proj
    GIT_REPOSITORY /home/deniz/work/dze/memory ##https://github.com/dzzze/memory.git
    GIT_TAG 29a15f40dc52270fbd2117917dbf601059354ae5
    GIT_SHALLOW true
    PREFIX "${thirdparty_prefix}/${proj_name}"
    SOURCE_DIR "${thirdparty_prefix}/${proj_name}/source"
    BINARY_DIR "${thirdparty_binary_dir}/${proj_name}/bin")

add_subdirectory(${${proj_name}_proj_SOURCE_DIR} ${${proj_name}_proj_BINARY_DIR} EXCLUDE_FROM_ALL)

unset(proj_name)
