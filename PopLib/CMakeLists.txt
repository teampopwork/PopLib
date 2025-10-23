# CMakeLists.txt
project(PopLib)

file(GLOB_RECURSE ALL_FILES CONFIGURE_DEPENDS
    "*.cpp" "*.c"
    "*.h" "*.hpp"
)

# dirty stupid hack. probably enough to not do #ifdef at the start of the files
set(API_ALLOWED_FILES "")
if(FEATURE_DISCORD_RPC)
    list(APPEND API_ALLOWED_FILES
        "api/discord.cpp"
        "api/discord.hpp"
    )
endif()
if(FEATURE_STEAM_API)
    list(APPEND API_ALLOWED_FILES
        "api/steam.cpp"
        "api/steam.hpp"
    )
endif()

set(FILTERED_FILES "")
foreach(FILE_PATH ${ALL_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${FILE_PATH}")
    string(REPLACE "\\" "/" REL_PATH "${REL_PATH}") # normalize for Windows

	# fuck u
    if(REL_PATH MATCHES "^todlib/toddrawtriangleinc\\.cpp$" OR
       REL_PATH MATCHES "^todlib/toddrawtriangle\\.cpp$")
        continue()
    endif()

    if(REL_PATH MATCHES "^graphics/SWTri/SWTri\\.(cpp|h)$")
        list(APPEND FILTERED_FILES "${FILE_PATH}")
    elseif(REL_PATH MATCHES "^graphics/SWTri/")
        continue()
    elseif(REL_PATH MATCHES "^api/")
        list(FIND API_ALLOWED_FILES "${REL_PATH}" FILE_INDEX)
        if(NOT FILE_INDEX EQUAL -1)
            list(APPEND FILTERED_FILES "${FILE_PATH}")
        endif()
        continue()
    else()
        list(APPEND FILTERED_FILES "${FILE_PATH}")
    endif()
endforeach()

set(ALL_FILES ${FILTERED_FILES})

set(SOURCE_FILES "")
set(HEADER_FILES "")

foreach(FILE_PATH ${ALL_FILES})
    get_filename_component(EXT "${FILE_PATH}" EXT)
    string(TOLOWER "${EXT}" EXT)

    if(EXT STREQUAL ".cpp" OR EXT STREQUAL ".c" OR EXT STREQUAL ".cxx")
        list(APPEND SOURCE_FILES "${FILE_PATH}")
    elseif(EXT STREQUAL ".h" OR EXT STREQUAL ".hpp")
        list(APPEND HEADER_FILES "${FILE_PATH}")
    endif()
endforeach()

function(capitalize_path INPUT OUTPUT)
    set(RESULT "")
    string(REPLACE "\\" "/" INPUT_PATH "${INPUT}")
    string(REPLACE "/" ";" PATH_PARTS "${INPUT_PATH}")
    foreach(PART ${PATH_PARTS})
        # handle exceptions
        string(TOLOWER "${PART}" LOWER_PART)
        if(LOWER_PART STREQUAL "imagelib")
            set(PART "ImageLib")
        elseif(LOWER_PART STREQUAL "paklib")
            set(PART "PakLib")
		elseif(LOWER_PART STREQUAL "todlib")
            set(PART "Todd Foley Lib")
        elseif(LOWER_PART STREQUAL "imgui")
            set(PART "Dear ImGui")
        elseif(LOWER_PART STREQUAL "readwrite")
            set(PART "ReadWrite")
        elseif(LOWER_PART STREQUAL "api")
            set(PART "APIs")
        elseif(LOWER_PART STREQUAL "chipmunk")
            set(PART "Chipmunk Physics")
        else()
            string(SUBSTRING "${PART}" 0 1 FIRST_CHAR)
            string(SUBSTRING "${PART}" 1 -1 REST_CHARS)
            string(TOUPPER "${FIRST_CHAR}" FIRST_CHAR)
            set(PART "${FIRST_CHAR}${REST_CHARS}")
        endif()
        string(APPEND RESULT "${PART}/")
    endforeach()
    string(REGEX REPLACE "/$" "" RESULT "${RESULT}")
    set(${OUTPUT} "${RESULT}" PARENT_SCOPE)
endfunction()

# group source files
foreach(FILE_PATH ${SOURCE_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${FILE_PATH}")
    get_filename_component(DIR_PATH "${REL_PATH}" PATH)
    capitalize_path("${DIR_PATH}" CAPITALIZED_GROUP)
    source_group("Source Files/${CAPITALIZED_GROUP}" FILES "${FILE_PATH}")
endforeach()

# group header files
foreach(FILE_PATH ${HEADER_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${FILE_PATH}")
    get_filename_component(DIR_PATH "${REL_PATH}" PATH)
    capitalize_path("${DIR_PATH}" CAPITALIZED_GROUP)
    source_group("Header Files/${CAPITALIZED_GROUP}" FILES "${FILE_PATH}")
endforeach()

set(ALL_GROUPED_FILES ${SOURCE_FILES} ${HEADER_FILES})

add_library(${PROJECT_NAME} STATIC ${ALL_GROUPED_FILES})

# hacks!!
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${POPLIB_ROOT_DIR}/external/stb_image
		${OPENMPT_INCLUDE_DIRS}
)

if (FEATURE_STEAM_API)
    if (NOT EXISTS "${STEAM_SDK_DIR}/public/steam/steam_api.h")
        message(FATAL_ERROR
            "Steamworks SDK not found in ${STEAM_SDK_DIR}.\n"
            "Please download the SDK from https://partner.steamgames.com/downloads/ "
            "and copy the 'sdk' folder into ${CMAKE_SOURCE_DIR}/private/steam/"
        )
    endif()

    target_include_directories(${PROJECT_NAME}
		PUBLIC
        	"${STEAM_SDK_DIR}/public"
    )

	target_link_libraries(${PROJECT_NAME} PUBLIC "${STEAMAPI_PATH}")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
	STB_IMAGE_IMPLEMENTATION
	STBI_MSC_SECURE_CRT
	STB_IMAGE_WRITE_IMPLEMENTATION
)

if (NOT WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
    )
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
    SDL3::SDL3-static
    SDL3_ttf::SDL3_ttf
    OpenAL::OpenAL
    miniaudio
    ogg
    Vorbis::vorbisfile
    Vorbis::vorbis
    CURL::libcurl
    zlibstatic
    glm
    misc
	lua_static
	${OPENMPT_LIBRARIES}
)

if (FEATURE_DISCORD_RPC)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        discord-rpc
    )
endif()


# Doxygen
find_package(Doxygen)

if (DOXYGEN_FOUND)
    message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

    set(DOXYGEN_INPUT_DIR .) # hack
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/docs/doxygen)

    set(DOXYGEN_CONFIG_FILE_IN ${CMAKE_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_CONFIG_FILE ${CMAKE_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_CONFIG_FILE_IN} ${DOXYGEN_CONFIG_FILE} @ONLY)
    file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

    add_custom_target(doc_doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating documentation"
        VERBATIM
    )
else()
    message(STATUS "Doxygen not found, documentation target will not be available")
endif()
