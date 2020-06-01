# `path` is relative to the current source directory.
function(make_target_name path)
    get_filename_component(dir ${path} DIRECTORY)
    get_filename_component(name ${path} NAME_WE)

    if (dir)
        string(REPLACE "/" "-" target_name "${PROJECT_NAME}-test-${dir}-${name}")
        set(target_name ${target_name} PARENT_SCOPE)
    else ()
        set(target_name "${PROJECT_NAME}-test-${name}" PARENT_SCOPE)
    endif ()
endfunction()

set(
    tests
    assignment.cpp
    constructors.cpp
    emplace.cpp
    in_place.cpp
    make_optional.cpp
    noexcept.cpp
    observers.cpp
    relops.cpp
    type_traits.cpp
    optional_reference.cpp
    ${concurrency_tests})

include(add_custom_test)
include(Catch2)

foreach (test ${tests})
    make_target_name(${test})

    add_executable(${target_name} ${test})
    target_link_libraries(${target_name} Catch2::Main dze::optional)
    add_custom_test(
        NAME ${target_name}
        COMMAND ${target_name}
        DEPENDS ${target_name})
endforeach ()
