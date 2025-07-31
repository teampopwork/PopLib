project(MISC)

add_library(misc STATIC aes.c glad/gl.c) #Please if someone sees this. PLEASE COME WITH A BETTER METHOD.
target_include_directories(misc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})