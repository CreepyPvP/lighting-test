cmake_minimum_required(VERSION 3.25)

project(
	lighting-test
)

set(ROOT_DIR ${PROJECT_SOURCE_DIR})

add_definitions( -DSUPPORT_FILEFORMAT_JPG -DSUPPORT_FILEFORMAT_HDR )

set(RAYLIB_DIR raylib)
add_subdirectory(${RAYLIB_DIR})

file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/code/*.cpp
)

add_executable(${PROJECT_NAME} 
    ${SOURCE_FILES}
)

target_link_libraries(${PROJECT_NAME} 
	raylib
)
