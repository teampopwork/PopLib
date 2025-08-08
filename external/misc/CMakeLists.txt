# CMakeLists.txt
project(misc)

set(SOURCES
	aes.c
	glad/gl.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})