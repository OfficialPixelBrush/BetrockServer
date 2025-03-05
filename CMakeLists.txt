cmake_minimum_required(VERSION 3.16.0)
cmake_policy(SET CMP0072 NEW)

project(BetrockServer VERSION 0.2.7)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Lua REQUIRED)

file(COPY "${CMAKE_SOURCE_DIR}/src/scripts/"
DESTINATION "${CMAKE_BINARY_DIR}/scripts")

# Find Git
find_package(Git REQUIRED)

# Get the latest Git commit hash
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the current Git branch name
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Configure the version header file
configure_file(
    ${CMAKE_SOURCE_DIR}/src/misc/version.h.in
    ${CMAKE_SOURCE_DIR}/src/misc/version.h
)

configure_file(
    ${CMAKE_SOURCE_DIR}/.github/ISSUE_TEMPLATE/bug_report.md.in
    ${CMAKE_SOURCE_DIR}/.github/ISSUE_TEMPLATE/bug_report.md
    @ONLY
)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/pixnbt"
    "${PROJECT_SOURCE_DIR}/src/coms"
    "${PROJECT_SOURCE_DIR}/src/enums"
    "${PROJECT_SOURCE_DIR}/src/logger"
    "${PROJECT_SOURCE_DIR}/src/misc/helpers"
    "${PROJECT_SOURCE_DIR}/src/misc/commands"
    "${PROJECT_SOURCE_DIR}/src/misc"
    "${PROJECT_SOURCE_DIR}/src/objects"
    "${PROJECT_SOURCE_DIR}/src/plugins"
    "${PROJECT_SOURCE_DIR}/src/plugins/terrain"
    "${PROJECT_SOURCE_DIR}/src/"
    ${LUA_INCLUDE_DIR}
)

add_executable(
    BetrockServer

    include/pixnbt/nbt.cpp
    include/pixnbt/tags/tag.cpp

    src/coms/coms.cpp
    src/coms/responses.cpp

    src/enums/blocks.cpp
    src/enums/labels.cpp

    src/logger/logger.cpp
    src/logger/style.cpp

    src/misc/helpers/datatypes.cpp
    src/misc/helpers/helper.cpp
    src/misc/helpers/luahelper.cpp

    src/misc/commands/command.cpp
    src/misc/gamerules.cpp
    src/misc/lighting.cpp
    src/misc/sysinfo.cpp

    src/objects/entity.cpp
    src/objects/player.cpp
    src/objects/world.cpp
    src/objects/worldManager.cpp

    src/plugins/plugins.cpp
    src/plugins/terrain/generator.cpp

    src/client.cpp
    src/config.cpp
    src/debug.cpp
    src/main.cpp
    src/server.cpp
)

# this is only defined to expose jthreads
target_compile_definitions(BetrockServer PRIVATE -D_LIBCPP_ENABLE_EXPERIMENTAL=1)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(BetrockServer PRIVATE -ggdb)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif ()

set(CMAKE_SYSTEM_NAME Linux)

# Maybe get Windows Target working
#if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
#    set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
#    set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
#    set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
#    set(CMAKE_RANLIB i686-w64-mingw32-ranlib)
#endif()

target_link_libraries(
    BetrockServer
    deflate
    ${LUA_LIBRARIES}
)

# Detect CPU architecture
if(CMAKE_SYSTEM_PROCESSOR)
    set(ARCH_SUFFIX "${CMAKE_SYSTEM_PROCESSOR}")
else()
    set(ARCH_SUFFIX "unknown")
endif()

# Set up CPack for AppImage
set(CPACK_PACKAGE_NAME "BetrockServer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BetrockServer - Minecraft Beta 1.7.3 Server-Engine")
set(CPACK_PACKAGE_VENDOR "Torben J. Virtmann")
set(CPACK_PACKAGE_CONTACT "letsplaytvirtmann@gmail.com")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Override the default package file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${ARCH_SUFFIX}")

include(CPack)

# Install the Betrock executable
install(TARGETS BetrockServer DESTINATION bin)

