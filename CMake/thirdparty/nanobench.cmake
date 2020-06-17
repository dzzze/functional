include_guard(GLOBAL)

include(fetch_content)
include(thirdparty_common)

set(proj_name nanobench)

fetch_content(
    ${proj_name}_proj
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG c534992696b9341274c6714931d0064d74239fcb
    GIT_SHALLOW true
    PREFIX "${thirdparty_prefix}/${proj_name}"
    SOURCE_DIR "${thirdparty_prefix}/${proj_name}/source"
    BINARY_DIR "${thirdparty_binary_dir}/${proj_name}/bin")

add_library(nanobench OBJECT ${CMAKE_CURRENT_LIST_DIR}/nanobench_impl.cpp)
target_include_directories(nanobench PUBLIC ${${proj_name}_proj_SOURCE_DIR}/src/include)
