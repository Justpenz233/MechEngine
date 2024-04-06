cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

project(ProjectLib)

file(GLOB_RECURSE SOURCE_FILES Source/*.cpp)
file(GLOB_RECURSE HEADER_FILES Source/*.h)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

add_library(ProjectLib SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(ProjectLib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_target_properties(ProjectLib PROPERTIES PARSE_HEADERS "${HEADER_FILES}")
target_include_directories(ProjectLib PUBLIC Source/)
target_link_libraries(ProjectLib PUBLIC MechEngineRuntime)