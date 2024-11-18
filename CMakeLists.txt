cmake_minimum_required(VERSION 3.20)
project(alear VERSION 1.0.0 LANGUAGES CXX ASM)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_C_STANDARD 99)

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin> CACHE INTERNAL "")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(ppu)

file(GLOB_RECURSE SRC_FILES 
    "${PROJECT_SOURCE_DIR}/code/PrxLib/**.cpp"
    "${PROJECT_SOURCE_DIR}/code/CoreLib/**.cpp"
    "${PROJECT_SOURCE_DIR}/code/CWLib/**.cpp"
    "${PROJECT_SOURCE_DIR}/code/Alear/**.cpp"
    "${PROJECT_SOURCE_DIR}/code/main.cpp"
)

set(ENABLE_LIBSM64 0)
set(ENABLE_CINEMACHINE 1)

set(SM64_SRC_FILES "")
if (ENABLE_LIBSM64)
    file(GLOB_RECURSE SM64_SRC_FILES "${PROJECT_SOURCE_DIR}/code/SM64/**.cpp")
endif()

file(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/code/**.h")
file(GLOB_RECURSE ASSEMBLY_FILES "${PROJECT_SOURCE_DIR}/code/**.s")

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${SM64_SRC_FILES} ${ASSEMBLY_FILES} ${HEADER_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/code/Alear
    ${PROJECT_SOURCE_DIR}/code/SM64
    ${PROJECT_SOURCE_DIR}/code/CWLib
    ${PROJECT_SOURCE_DIR}/code/Spuge
    ${PROJECT_SOURCE_DIR}/code/CoreLib
    ${PROJECT_SOURCE_DIR}/code/PrxLib
    ${PROJECT_SOURCE_DIR}/code

    ${PROJECT_SOURCE_DIR}/sdk/fmod
    ${PROJECT_SOURCE_DIR}/sdk/libsm64/include

    ${PS3_SDK}/target/common/include
	${PS3_SDK}/target/ppu/include
)

file(STRINGS "${PROJECT_SOURCE_DIR}/symbols.yml" SYMBOLS_FILE)
while(SYMBOLS_FILE)
    # Some regex like (^[^\s]*):(\s*.*)[^\s] maybe?
    list(POP_FRONT SYMBOLS_FILE LINE)
    string(REPLACE ": " ";" LIST ${LINE})
    list(GET LIST 0 SYMBOL_NAME)
    list(GET LIST 1 SYMBOL_VALUE)

    target_link_options(${CMAKE_PROJECT_NAME} PUBLIC "-Wl,--defsym,${SYMBOL_NAME}=${SYMBOL_VALUE}")
endwhile()

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC
    $<$<COMPILE_LANGUAGE:C,CXX>:
        #-Wall
        #-Wextra
        -Os
        -fno-exceptions
        -fno-strict-aliasing
        -ffunction-sections
        -fdata-sections
        -fmodulo-sched
        #-v
    >

    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-rtti
    >
)

target_link_libraries(${CMAKE_PROJECT_NAME} fs_stub audio_stub gcm_cmd http_stub http_util_stub)
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC 
    ALEAR_MAJOR_VERSION=${CMAKE_PROJECT_VERSION_MAJOR} 
    ALEAR_MINOR_VERSION=${CMAKE_PROJECT_VERSION_MINOR} 
    SCRIPT_DEBUG=1

    __GCC__=1
    _NO_EX=1
    _HAS_EXCEPTIONS=0
)

target_precompile_headers(${CMAKE_PROJECT_NAME} PUBLIC 
    code/CoreLib/mmtypes.h
)

if(ENABLE_CINEMACHINE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC __CINEMACHINE__=1)
endif()

if (ENABLE_LIBSM64)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC __SM64__=1)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/sdk/libsm64/libsm64.a)
endif()

# Create the sprx
include(sprx)