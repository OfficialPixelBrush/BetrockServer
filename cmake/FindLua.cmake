# FindLua.cmake - Custom Lua finder for vcpkg on Windows
# This module finds Lua and sets the LUA_INCLUDE_DIR and LUA_LIBRARIES variables

if(LUA_FOUND)
    return()
endif()

# First, check if VCPKG_ROOT is available (set by vcpkg toolchain)
if(DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_ROOT "$ENV{VCPKG_ROOT}")
elseif(DEFINED VCPKG_ROOT)
    # VCPKG_ROOT already set
else()
    # Default vcpkg location
    set(VCPKG_ROOT "$ENV{USERPROFILE}/vcpkg")
endif()

# Detect architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_LUA_ARCH "x64-windows")
else()
    set(_LUA_ARCH "x86-windows")
endif()

# Try to find Lua in vcpkg packages
set(_LUA_VCPKG_PATH "${VCPKG_ROOT}/packages/lua_${_LUA_ARCH}")

if(EXISTS "${_LUA_VCPKG_PATH}/include/lua.h")
    message(STATUS "Found Lua in vcpkg at: ${_LUA_VCPKG_PATH}")
    
    set(LUA_INCLUDE_DIR "${_LUA_VCPKG_PATH}/include")
    
    # Find the library file
    find_library(LUA_LIBRARIES
        NAMES lua lua54
        PATHS "${_LUA_VCPKG_PATH}/lib"
        NO_DEFAULT_PATH
        REQUIRED
    )
    
    set(LUA_FOUND TRUE)
    set(LUA_VERSION_STRING "5.4")
    
    message(STATUS "Lua include directory: ${LUA_INCLUDE_DIR}")
    message(STATUS "Lua libraries: ${LUA_LIBRARIES}")
else()
    # Fallback to system Lua search
    find_package(Lua QUIET CONFIG)
    if(NOT LUA_FOUND)
        message(FATAL_ERROR "Could not find Lua. Please install it via vcpkg or system package manager.")
    endif()
endif()

# Mark as found
set(LUA_FOUND TRUE CACHE BOOL "Lua found" FORCE)
set(LUA_INCLUDE_DIR "${LUA_INCLUDE_DIR}" CACHE PATH "Lua include directory" FORCE)
set(LUA_LIBRARIES "${LUA_LIBRARIES}" CACHE STRING "Lua libraries" FORCE)
