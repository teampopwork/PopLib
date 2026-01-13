# CMakeLists.txt
project(Tetris)

set(SOURCES
	# Sources
	main.cpp
	gameapp.cpp
	board.cpp
	../res.cpp

	# Headers
	gameapp.hpp
	board.hpp
	../res.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
	../
	${POPLIB_ROOT_DIR}
	${POPLIB_ROOT_DIR}/PopLib/ # common.hpp
)

if (NOT WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
    )
endif()

target_link_libraries(${PROJECT_NAME} PopLib)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${POPLIB_ROOT_DIR}/examples/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${POPLIB_ROOT_DIR}/examples/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${POPLIB_ROOT_DIR}/examples/bin"
    RUNTIME_OUTPUT_NAME ${PROJECT_NAME}
)

include(${POPLIB_ROOT_DIR}/cmake/CopyDLLPost.cmake)
if (FEATURE_STEAM_API)
	copy_dll_post(${PROJECT_NAME} ${STEAMAPI_PATH})
endif()
if(WIN32)
    set(LIBOPENMPT_PATH "${OPENMPT_BIN_DIR}/${OPENMPT_LIB_NAME}.dll")
    set(OPENMPT_PLUGIN_PATH "${OPENMPT_BIN_DIR}/openmpt")
    copy_dll_post(${PROJECT_NAME} "${LIBOPENMPT_PATH}")
	copy_dll_post(${PROJECT_NAME} "${OPENMPT_PLUGIN_PATH}-mpg123.dll")
	copy_dll_post(${PROJECT_NAME} "${OPENMPT_PLUGIN_PATH}-ogg.dll")
	copy_dll_post(${PROJECT_NAME} "${OPENMPT_PLUGIN_PATH}-vorbis.dll")
	copy_dll_post(${PROJECT_NAME} "${OPENMPT_PLUGIN_PATH}-zlib.dll")
endif()