cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

project(Examples)

file(GLOB_RECURSE SOURCE_FILES Source/*.cpp)
file(GLOB_RECURSE HEADER_FILES Source/*.h)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

add_library(Examples SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(Examples PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_target_properties(Examples PROPERTIES PARSE_HEADERS "${HEADER_FILES}")
target_include_directories(Examples PUBLIC Source/)
target_link_libraries(Examples PUBLIC MechEngineRuntime)