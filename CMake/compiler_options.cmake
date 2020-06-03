set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Let's not open this can of worms.
if (NOT ${CMAKE_C_COMPILER_ID} STREQUAL ${CMAKE_CXX_COMPILER_ID})
    message(FATAL_ERROR "Different compilers for C and C++ is not supported.")
endif ()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" OR
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")

    add_compile_options(-Wall -Wextra -Wmissing-declarations -Wswitch-enum -Werror -pedantic-errors)
    add_compile_options($<$<CXX_COMPILER_ID:Clang>:-Wdeprecated>)
    add_compile_options($<$<CXX_COMPILER_ID:GNU>:-Wshadow=compatible-local>)
    add_compile_options($<$<CONFIG:Debug>:-g3>)

    add_compile_options($<$<AND:$<PLATFORM_ID:Darwin>,$<CXX_COMPILER_ID:GNU>>:-fvisibility=hidden>)
    add_link_options($<$<AND:$<PLATFORM_ID:Darwin>,$<CXX_COMPILER_ID:GNU>>:-fvisibility=hidden>)

    option(${PROJECT_NAME}_sanitize_address "Enable address sanitizer" OFF)
    add_compile_options($<$<BOOL:${${PROJECT_NAME}_sanitize_address}>:-fsanitize=address>)
    add_compile_options($<$<BOOL:${${PROJECT_NAME}_sanitize_address}>:-fno-sanitize-recover=address>)
    add_link_options($<$<BOOL:${${PROJECT_NAME}_sanitize_address}>:-fsanitize=address>)

    option(${PROJECT_NAME}_sanitize_ub "Enable undefined behaviour sanitizer" OFF)
    add_compile_options($<$<BOOL:${${PROJECT_NAME}_sanitize_ub}>:-fsanitize=undefined>)
    add_compile_options($<$<BOOL:${${PROJECT_NAME}_sanitize_ub}>:-fno-sanitize-recover=undefined>)
    add_link_options($<$<BOOL:${${PROJECT_NAME}_sanitize_ub}>:-fsanitize=undefined>)

    option(${PROJECT_NAME}_use_lld "Use the LLVM linker" OFF)
    add_link_options($<$<BOOL:${${PROJECT_NAME}_use_lld}>:-fuse-ld=lld>)
endif ()
