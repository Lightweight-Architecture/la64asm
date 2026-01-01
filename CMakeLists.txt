cmake_minimum_required(VERSION 3.16)

project(LA64ASM LANGUAGES C)

include(FetchContent)

FetchContent_Declare(
    la64
    GIT_REPOSITORY https://github.com/lightweight-architecture/la64.git
    GIT_TAG main
)

FetchContent_Declare(
    lautils
    GIT_REPOSITORY https://github.com/lightweight-architecture/lautils.git
    GIT_TAG main
)

FetchContent_MakeAvailable(la64)
FetchContent_MakeAvailable(lautils)

add_executable(la64asm
    src/main.c
    src/cmptok.c
    src/code.c
    src/compile.c
    src/compiler.c
    src/label.c
    src/opcode.c
    src/register.c
    src/section.c
)

target_include_directories(la64asm
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(la64asm
    PRIVATE la64_headers
    PRIVATE lautils
)

target_compile_features(la64asm PRIVATE c_std_99)