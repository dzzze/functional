include(FetchContent)

# Wrapper around FetchContent_Populate.
# Does not call populate when the git tag of the checked out code is
# the same as the one passed to the function.
function(fetch_content proj_name)
    set(options "")
    set(one_value_args GIT_TAG SOURCE_DIR BINARY_DIR)
    set(multi_value_args "")
    cmake_parse_arguments(
        ${proj_name}
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN})

    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY "${${proj_name}_SOURCE_DIR}"
        OUTPUT_VARIABLE checked_out_tag)

    if ((NOT checked_out_tag) OR (NOT ${checked_out_tag} MATCHES "^${${proj_name}_GIT_TAG}.*"))
        FetchContent_Populate(${proj_name} ${ARGN})
    endif ()

    set(${proj_name}_POPULATED True PARENT_SCOPE)
    set(${proj_name}_SOURCE_DIR "${${proj_name}_SOURCE_DIR}" PARENT_SCOPE)
    set(${proj_name}_BINARY_DIR "${${proj_name}_BINARY_DIR}" PARENT_SCOPE)
endfunction()
