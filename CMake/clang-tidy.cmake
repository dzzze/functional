find_program(
    clang_tidy
    NAMES clang-tidy
    DOC "Path to clang-tidy executable")

if (NOT clang_tidy)
    message(FATAL_ERROR "Could not find clang-tidy.")
else ()
    # Generate commands because database because Clang-Tidy cannot always get include
    # directories etc.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    # Force CMake to re-run if .clang-tidy is modified. And, make configuration available
    # to genererated files.
    configure_file(${PROJECT_SOURCE_DIR}/.clang-tidy ${PROJECT_BINARY_DIR} COPYONLY)
    # Provide definitions for targets that require a rebuild once .clang-tidy changes.
    file(SHA256 ${PROJECT_SOURCE_DIR}/.clang-tidy clang_tidy_hash)
    add_compile_definitions("CLANG_TIDY_HASH=${clang_tidy_hash}")
    unset(clang_tidy_hash)

    set(clang_tidy_options -quiet)
    # Clang and GCC warning options differ. This makes Clang ignore
    # the options that are specific to GCC.
    if (${CMAKE_C_COMPILER_ID} STREQUAL GNU AND ${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        list(APPEND clang_tidy_options -extra-arg=-Wno-unknown-warning-option)
    endif ()
    set(CMAKE_C_CLANG_TIDY ${clang_tidy} ${clang_tidy_options})
    set(CMAKE_CXX_CLANG_TIDY ${clang_tidy} ${clang_tidy_options})
endif ()
