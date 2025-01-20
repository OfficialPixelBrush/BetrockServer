cmake_minimum_required(VERSION 3.16.0)
cmake_policy(SET CMP0072 NEW)
set (CMAKE_CXX_STANDARD 20)
project(BetrockServer VERSION 0.1.1)

add_compile_options(
  "-ggdb"
)

# Configure the version header file
configure_file(${CMAKE_SOURCE_DIR}/src/version.h.in ${CMAKE_SOURCE_DIR}/src/version.h)

add_executable(
    BetrockServer
    src/main.cpp
    src/world.cpp
    src/debug.cpp
)

set(CMAKE_SYSTEM_NAME Linux)

# TODO: Get Windows Target working
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
    set(CMAKE_RANLIB i686-w64-mingw32-ranlib)
    target_link_libraries(
        BetrockServer
        deflate
    )
else()
    # Apply AddressSanitizer to all build types
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
    target_link_libraries(
        BetrockServer
        deflate
    )
endif()
