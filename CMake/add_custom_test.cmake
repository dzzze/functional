function(add_custom_test)
    set(options "")
    set(one_value_args NAME)
    set(multi_value_args DEPENDS CUSTOM_TARGET_DEPENDS)
    cmake_parse_arguments(
        ARG
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN})

    foreach (dep ${ARG_DEPENDS})
        list(APPEND deps ${dep})
    endforeach ()

    foreach (dep ${ARG_CUSTOM_TARGET_DEPENDS})
        list(APPEND deps ${dep})
        list(APPEND deps "$<TARGET_PROPERTY:${dep},dependent_files>")
    endforeach ()

    if (ARG_NAME STREQUAL "")
        message(FATAL_ERROR "add_custom_test called without argument for name")
    endif ()

    set(name ${ARG_NAME})

    add_test(NAME ${name} ${ARG_UNPARSED_ARGUMENTS})

    set(stamp_file "${CMAKE_CURRENT_BINARY_DIR}/${name}_stamp")

    add_custom_command(
        OUTPUT ${stamp_file}
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --progress -R '^${name}$$'
        COMMAND ${CMAKE_COMMAND} -E touch ${stamp_file}
        USES_TERMINAL
        COMMENT "Running test ${name}"
        DEPENDS ${deps})

    add_custom_target(
        ${PROJECT_NAME}-run-custom-test-${name}
        DEPENDS ${stamp_file})

    set_target_properties(
        ${PROJECT_NAME}-run-custom-test-${name}
        PROPERTIES
        dependent_files ${stamp_file})

    if (TARGET ${PROJECT_NAME}-run-custom-tests)
        add_dependencies(${PROJECT_NAME}-run-custom-tests ${PROJECT_NAME}-run-custom-test-${name})
    endif ()
endfunction()
