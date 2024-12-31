# Make sure we've loaded the SDK
include("${CMAKE_CURRENT_LIST_DIR}/sdk.cmake")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR powerpc64)

link_directories("${PS3_SDK}/target/ppu/lib/fno-exceptions/fno-rtti" "${PS3_SDK}/target/ppu/lib/fno-exceptions" "${PS3_SDK}/target/ppu/lib")

set(CMAKE_C_COMPILER  "${PS3_SDK}/host-win32/ppu/bin/ppu-lv2-gcc.exe")
# set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_C_STANDARD_LIBRARIES "${PS3_SDK}/target/ppu/lib/fno-exceptions/fno-rtti/libc.a ${PS3_SDK}/target/ppu/lib/fno-exceptions/fno-rtti/libm.a ${PS3_SDK}/target/ppu/lib/liblv2_stub.a")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_COMPILER} -mprx <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_CXX_COMPILER "${PS3_SDK}/host-win32/ppu/bin/ppu-lv2-g++.exe")
# set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} ${PS3_SDK}/target/ppu/lib/fno-exceptions/fno-rtti/libstdc++.a")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_COMPILER} -mprx <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_ASM_COMPILER "${PS3_SDK}/host-win32/ppu/bin/ppu-lv2-as.exe")
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")


set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELEASE "")
set(CMAKE_ASM_FLAGS_RELEASE_INIT "")


set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_STATIC_LIBRARY_SUFFIX_C ".a")
set(CMAKE_STATIC_LIBRARY_SUFFIX_CXX ".a")

set(CMAKE_EXECUTABLE_SUFFIX ".prx")
set(CMAKE_EXECUTABLE_SUFFIX_C ".prx")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".prx")

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
SET(CMAKE_CROSSCOMPILING 1)

# CMake can't scan outside the PS3 SDK root
#set(CMAKE_SYSROOT "${PS3_SDK}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)