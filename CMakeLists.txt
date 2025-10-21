# CMakeLists.txt
cmake_minimum_required(VERSION 4.0.2 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PopLib LANGUAGES C CXX ASM)

set(POPLIB_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/cmake/CopyDLLPost.cmake)
include(FetchContent)

# temp workaround for me
set(_Python3_EXECUTABLE_DEBUG "" CACHE FILEPATH "Python debug executable")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(FEATURE_DISCORD_RPC "Feature Discord RPC" OFF)
option(FEATURE_STEAM_API "Feature Steam API" OFF)
option(BUILD_EXAMPLES "Build Examples" ON)
option(CONSOLE "Show the console on Windows" ON)
option(BUILD_TOOLS "Build Tools" ON)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Using x64")
	set(IS_64BIT true)
else ()
    message (STATUS "Using x86")
	set(IS_64BIT false)
endif()

if(FEATURE_DISCORD_RPC)
	add_compile_definitions(POP_FEATURE_DISCORD_RPC)
endif()

if(FEATURE_STEAM_API)
	add_compile_definitions(POP_FEATURE_STEAM_API)
endif()

if(CONSOLE AND WIN32)
	add_compile_definitions(POP_CONSOLE_ON)
endif()

if(FEATURE_STEAM_API)
    set(STEAM_SDK_DIR "${CMAKE_SOURCE_DIR}/private/steam/sdk" CACHE FILEPATH "Path to Steam API dir" FORCE)

    if (WIN32)
        if (IS_64BIT)
            message(STATUS "Using Steam API x64 for Windows")
            set(STEAMAPI_PATH "${STEAM_SDK_DIR}/redistributable_bin/win64/steam_api64.lib" CACHE FILEPATH "Path to Steam API lib" FORCE)
        else()
            message(STATUS "Using Steam API x86 for Windows")
            set(STEAMAPI_PATH "${STEAM_SDK_DIR}/redistributable_bin/steam_api.lib" CACHE FILEPATH "Path to Steam API lib" FORCE)
        endif()
    elseif (APPLE)
        message(STATUS "Using Steam API for macOS")
        set(STEAMAPI_PATH "${STEAM_SDK_DIR}/redistributable_bin/osx/libsteam_api.dylib" CACHE FILEPATH "Path to Steam API lib" FORCE)
    elseif (UNIX)
        if (IS_64BIT)
            message(STATUS "Using Steam API x64 for Linux")
            set(STEAMAPI_PATH "${STEAM_SDK_DIR}/redistributable_bin/linux64/libsteam_api.so" CACHE FILEPATH "Path to Steam API lib" FORCE)
        else()
            message(STATUS "Using Steam API x86 for Linux")
            set(STEAMAPI_PATH "${STEAM_SDK_DIR}/redistributable_bin/linux32/libsteam_api.so" CACHE FILEPATH "Path to Steam API lib" FORCE)
        endif()
    endif()
endif()

# external libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build as DLL" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(MINIAUDIO_NO_LIBVORBIS ON CACHE BOOL "Disable miniaudio_libvorbis" FORCE) 
set(MINIAUDIO_NO_LIBOPUS ON CACHE BOOL "Disable miniaudio_libopus" FORCE) 

#-- openal
set(LIBTYPE "STATIC" CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
#-- end openal

#-- lua
set(LUA_ENABLE_TESTING OFF CACHE BOOL "Disable Lua tests")
set(LUA_ENABLE_SHARED OFF CACHE BOOL "Build static Lua library")
#-- end lua

#-- curl
set(BUILD_CURL_EXE OFF CACHE BOOL "Build curl executable" FORCE)
set(CURL_ZLIB OFF CACHE BOOL "Disable curl's use of system zlib" FORCE)
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "Use libpsl" FORCE)
set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
set(CURL_USE_MBEDTLS OFF CACHE BOOL "" FORCE)
#-- end curl

#-- libopenmpt
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # x64
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
            set(LIBOPENMPT_ARCH "amd64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
            set(LIBOPENMPT_ARCH "arm64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64EC")
            set(LIBOPENMPT_ARCH "arm64ec")
        else()
            message(FATAL_ERROR "Unsupported Windows architecture: ${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    else()
        set(LIBOPENMPT_ARCH "x86")
    endif()

    set(OPENMPT_LIB_DIR "${CMAKE_SOURCE_DIR}/external/libopenmpt/lib/${LIBOPENMPT_ARCH}")
    set(OPENMPT_BIN_DIR "${CMAKE_SOURCE_DIR}/external/libopenmpt/bin/${LIBOPENMPT_ARCH}")

    include_directories("${CMAKE_SOURCE_DIR}/external/libopenmpt/include")
    link_directories("${OPENMPT_LIB_DIR}")

    set(OPENMPT_LIB_NAME "libopenmpt")
    set(OPENMPT_LIBRARIES "${OPENMPT_LIB_DIR}/${OPENMPT_LIB_NAME}.lib")

    set(OPENMPT_FOUND TRUE)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(OPENMPT libopenmpt)

	link_directories(${OPENMPT_LIBRARY_DIRS})
endif()

if(NOT OPENMPT_FOUND)
    message(FATAL_ERROR "libopenmpt not found")
endif()

message(STATUS "libopenmpt found: includes=${OPENMPT_INCLUDE_DIRS} libs=${OPENMPT_LIBRARIES}")
#-- end libopenmpt

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG main
)

FetchContent_Declare(
    SDL3_TTF
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
    GIT_TAG main
)

FetchContent_Declare(
    Lua
    GIT_REPOSITORY https://github.com/walterschell/Lua
    GIT_TAG master
)

# the most hackiest hacks i've ever done
if(APPLE)
    find_library(OPENAL_LIBRARY OpenAL REQUIRED)
    
    if(NOT OPENAL_LIBRARY)
        message(FATAL_ERROR "System OpenAL framework not found")
    endif()
    
    add_library(OpenAL::OpenAL INTERFACE IMPORTED)
    set_target_properties(OpenAL::OpenAL PROPERTIES
        INTERFACE_LINK_LIBRARIES "${OPENAL_LIBRARY}"
    )
    
    message(STATUS "Using system OpenAL framework: ${OPENAL_LIBRARY}")
else()
    FetchContent_Declare(
		OpenAL
		GIT_REPOSITORY https://github.com/teampopwork/openal-soft
		GIT_TAG master
	)
    FetchContent_MakeAvailable(OpenAL)
endif()

FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio
    GIT_TAG master
)

FetchContent_Declare(
    ogg
    GIT_REPOSITORY https://github.com/teampopwork/ogg
    GIT_TAG master
)

FetchContent_Declare(
    vorbis
    GIT_REPOSITORY https://github.com/teampopwork/vorbis
    GIT_TAG master
)

FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl
    GIT_TAG master
)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib
    GIT_TAG master
)

FetchContent_Declare(
    discordrpc
    GIT_REPOSITORY https://github.com/EclipseMenu/discord-presence
    GIT_TAG main
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG master
)

FetchContent_MakeAvailable(SDL3 SDL3_TTF miniaudio ogg vorbis curl zlib discordrpc glm Lua)

add_subdirectory(external/misc EXCLUDE_FROM_ALL)
add_subdirectory(PopLib)

if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

# djugjsfgufdgujdfgiujgdijfgifjdgidfjgifdgjfdgufdguifdg electr0gunner told me to add this
if(BUILD_EXAMPLES OR BUILD_TOOLS)
    set(demo_deps PopLib)

    if(BUILD_EXAMPLES)
        list(APPEND demo_deps
            V14Demo PhysicsDemo Barebones
			Tetris
        )
    endif()

    add_custom_target(alldemos ALL DEPENDS ${demo_deps})
endif()
