cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(ENGINE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(ENGINE_SOURCE_DIR ${ENGINE_ROOT_DIR}/Source PARENT_SCOPE)
add_subdirectory(Source)