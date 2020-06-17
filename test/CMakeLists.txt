# `path` is relative to the current source directory.
function(make_target_names path)
    get_filename_component(dir ${path} DIRECTORY)
    get_filename_component(name ${path} NAME_WE)

    if (dir)
        string(REPLACE "/" "-" test_name "${dir}-${name}")
    else ()
        set(test_name ${name})
    endif ()

    set(test_name ${test_name} PARENT_SCOPE)
    set(exe_name "${PROJECT_NAME}-test-${test_name}" PARENT_SCOPE)
endfunction()

set(
    tests
    function.cpp)

include(add_custom_test)
include(thirdparty/Catch2)

foreach (test ${tests})
    make_target_names(${test})

    add_executable(${exe_name} ${test})
    target_link_libraries(${exe_name} Catch2::Main dze::functional)
    add_custom_test(
        NAME ${test_name}
        COMMAND $<TARGET_FILE:${exe_name}>
        DEPENDS ${exe_name})
endforeach ()
