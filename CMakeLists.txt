cmake_minimum_required(VERSION 3.16.0)
cmake_policy(SET CMP0072 NEW)
set (CMAKE_CXX_STANDARD 20)
project(BetrockServer VERSION 0.1.5)

add_compile_options(
  "-ggdb"
)

# Configure the version header file
configure_file(${CMAKE_SOURCE_DIR}/src/misc/version.h.in ${CMAKE_SOURCE_DIR}/src/misc/version.h)

include_directories(
    "${PROJECT_SOURCE_DIR}/src/coms"
    "${PROJECT_SOURCE_DIR}/src/enums"
    "${PROJECT_SOURCE_DIR}/src/objects"
    "${PROJECT_SOURCE_DIR}/src/misc"
    "${PROJECT_SOURCE_DIR}/src/misc/generators"
    "${PROJECT_SOURCE_DIR}/src/"
)

add_executable(
    BetrockServer

    src/coms/coms.cpp
    src/coms/responses.cpp

    src/enums/items.cpp
    
    src/misc/generators/generator.cpp
    src/misc/generators/worleyPeakGenerator.cpp

    src/misc/command.cpp
    src/misc/helper.cpp

    src/objects/world.cpp
    src/objects/entity.cpp
    src/objects/player.cpp

    src/main.cpp
    src/debug.cpp
    src/server.cpp
    src/client.cpp
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


include(CPack)

# Set up CPack for AppImage
set(CPACK_PACKAGE_NAME "BetrockServer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BetrockServer - Minecraft Beta 1.7.3 Server-Engine")
set(CPACK_PACKAGE_VENDOR "Torben J. Virtmann")
set(CPACK_PACKAGE_CONTACT "letsplaytvirtmann@gmail.com")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Install the Betrock executable
install(TARGETS BetrockServer DESTINATION bin)
