cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

project(ExampleMain)
set(ENGINE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Engine)
set(ENGINE_SOURCE_DIR ${ENGINE_ROOT_DIR}/Source)
set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(USER_PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Examples)
set(USER_PROJECT_SOURCE_DIR ${PROJECT_ROOT_DIR}/Source)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR}/${CMAKE_BUILD_TYPE})

add_subdirectory(BuildTool)

add_subdirectory(Examples)
set(USER_PROJECT_NAME Examples)

add_subdirectory(Engine)


add_executable(ExampleMain main.cpp)
target_link_libraries(ExampleMain PUBLIC MechEngineEditor)
target_link_libraries(ExampleMain INTERFACE MechEngineExamples)
target_compile_definitions(ExampleMain PRIVATE "PROJECT_DIR=\"${USER_PROJECT_ROOT_DIR}\"")