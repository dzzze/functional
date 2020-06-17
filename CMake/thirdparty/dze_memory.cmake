include_guard(GLOBAL)

include(fetch_content)
include(thirdparty_common)

set(proj_name dze_memory)

fetch_content(
    ${proj_name}_proj
    GIT_REPOSITORY /home/deniz/work/dze/memory #https://github.com/dzzze/type_traits.git
    GIT_TAG 480c5c9da19f5ab20f5ab0e2a8c903c6dc045d32
    GIT_SHALLOW true
    PREFIX "${thirdparty_prefix}/${proj_name}"
    SOURCE_DIR "${thirdparty_prefix}/${proj_name}/source"
    BINARY_DIR "${thirdparty_binary_dir}/${proj_name}/bin")

add_subdirectory(${${proj_name}_proj_SOURCE_DIR} ${${proj_name}_proj_BINARY_DIR} EXCLUDE_FROM_ALL)

unset(proj_name)
