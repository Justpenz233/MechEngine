cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
set(ENGINE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(ENGINE_SOURCE_DIR ${ENGINE_ROOT_DIR}/Source PARENT_SCOPE)
if (NOT TARGET ThirdParty)
    add_subdirectory(ThirdParty)
endif ()
add_subdirectory(Source)