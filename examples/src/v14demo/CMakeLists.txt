# CMakeLists.txt
project(V14Demo)

set(SOURCES
	# Sources
	main.cpp
	v14demoapp.cpp
	board.cpp
	demowidget.cpp

	# Headers
	v14demoapp.hpp
	board.hpp
	demowidget.hpp
	
	../res.hpp
	../res.cpp
)

if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
	../
	${POPLIB_ROOT_DIR}
	${POPLIB_ROOT_DIR}/PopLib/ # common.hpp
)

target_link_libraries(${PROJECT_NAME} PopLib)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${POPLIB_ROOT_DIR}/examples/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${POPLIB_ROOT_DIR}/examples/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${POPLIB_ROOT_DIR}/examples/bin"
    RUNTIME_OUTPUT_NAME ${PROJECT_NAME}
)

include(${POPLIB_ROOT_DIR}/cmake/CopyDLLPost.cmake)
copy_dll_post(${PROJECT_NAME} ${BASS_PATH})
if (FEATURE_STEAM_API)
	copy_dll_post(${PROJECT_NAME} ${STEAMAPI_PATH})
endif()