set(CMAKE_C_STANDARD 11 CACHE STRING "C standard version")
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard version")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckIPOSupported)
check_ipo_supported(LANGUAGES C CXX)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)

add_compile_options(-Wall -Wextra -Wmissing-declarations -Wswitch-enum -Werror -pedantic-errors)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>)
add_compile_options($<$<AND:$<C_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:GNU>>:-Wshadow=compatible-local>)
add_compile_options($<$<AND:$<C_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:Clang>>:-Wdeprecated>)
add_compile_options($<$<CONFIG:Debug>:-g3>)
add_compile_options($<$<BOOL:${HIKARI_SANITIZE_ADDRESS}>:-fsanitize=address>)
add_compile_options($<$<BOOL:${HIKARI_SANITIZE_ADDRESS}>:-fno-sanitize-recover=address>)
add_compile_options($<$<BOOL:${HIKARI_SANITIZE_UB}>:-fsanitize=undefined>)
add_compile_options($<$<BOOL:${HIKARI_SANITIZE_UB}>:-fno-sanitize-recover=undefined>)

add_link_options($<$<AND:$<C_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:Clang>>:--rtlib=compiler-rt>)
add_link_options($<$<AND:$<C_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:Clang>>:-lunwind>)
add_link_options($<$<BOOL:${HIKARI_SANITIZE_ADDRESS}>:-fsanitize=address>)
add_link_options($<$<BOOL:${HIKARI_SANITIZE_UB}>:-fsanitize=undefined>)

option(HIKARI_USE_LLD "Use the LLVM linker when available" ON)

find_program(lld_path ld.lld)

# Enable for Clang in all configurations.
# Enable for GCC in Debug only as LTO requires GNU ld.
if (HIKARI_USE_LLD AND lld_path)
    add_link_options($<$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:Clang>>:-fuse-ld=lld>)
    add_link_options($<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:GNU>>:-fuse-ld=lld>)
    add_link_options($<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:Clang>>:-fuse-ld=lld>)
    add_link_options($<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU>>:-fuse-ld=lld>)
endif ()
