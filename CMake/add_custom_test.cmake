include_guard(GLOBAL)

include(fetch_content)
include(thirdparty_common)

set(proj_name add_custom_test)

fetch_content(
    ${proj_name}_proj
    GIT_REPOSITORY https://github.com/dzzze/add_custom_test
    GIT_TAG 6533e7ef2a8fed7a9285ef62597ee74184040582
    GIT_SHALLOW true
    PREFIX "${thirdparty_prefix}/${proj_name}"
    SOURCE_DIR "${thirdparty_prefix}/${proj_name}/source")

include(${${proj_name}_proj_SOURCE_DIR}/add_custom_test.cmake)

unset(proj_name)
