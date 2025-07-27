# CMakeLists.txt
cmake_minimum_required(VERSION 4.0.2 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PopLib LANGUAGES C CXX ASM)

set(POPLIB_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

set(BUILD_SHARED_LIBS OFF)

# temp workaround for me
set(_Python3_EXECUTABLE_DEBUG "" CACHE FILEPATH "Python debug executable")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(FEATURE_DISCORD_RPC "Discord RPC" OFF)
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
	add_compile_definitions(_FEATURE_DISCORD_RPC)
endif()

if(CONSOLE AND WIN32)
	add_compile_definitions(_CONSOLE_ON)
endif()

if (IS_64BIT)
    message (STATUS "Getting the x64 BASS path")
	set(BASS_LIB_PATH ${POPLIB_ROOT_DIR}/external/bass/x64)
else ()
    message (STATUS "Getting the x86 BASS path")
	set(BASS_LIB_PATH  ${POPLIB_ROOT_DIR}/external/bass/x86)
endif()

if (WIN32)
    set(BASS_PATH "${BASS_LIB_PATH}/bass.dll" CACHE FILEPATH "Path to BASS DLL" FORCE)
elseif (UNIX)
    set(BASS_PATH "${BASS_LIB_PATH}/libbass.so" CACHE FILEPATH "Path to BASS SO" FORCE)
endif()

# external libraries

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build as DLL" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(MINIAUDIO_NO_LIBVORBIS ON CACHE BOOL "Disable miniaudio_libvorbis" FORCE) 
set(MINIAUDIO_NO_LIBOPUS ON CACHE BOOL "Disable miniaudio_libopus" FORCE) 
# OpenAL
set(LIBTYPE "STATIC" CACHE BOOL "" FORCE) 

add_subdirectory(external/misc EXCLUDE_FROM_ALL)

#curl stuff

set(BUILD_CURL_EXE OFF CACHE BOOL "Build curl executable" FORCE)
set(CURL_ZLIB OFF CACHE BOOL "Disable curl's use of system zlib" FORCE)
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "Use libpsl" FORCE)
set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
set(CURL_USE_MBEDTLS OFF CACHE BOOL "" FORCE)


include(FetchContent)

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
    OpenAL
    GIT_REPOSITORY https://github.com/teampopwork/openal-soft
    GIT_TAG master
)

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

FetchContent_MakeAvailable(SDL3 SDL3_TTF OpenAL miniaudio ogg vorbis curl zlib discordrpc)

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
