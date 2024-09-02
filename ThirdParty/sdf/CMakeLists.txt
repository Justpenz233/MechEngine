cmake_minimum_required(VERSION 3.3)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)
if (POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif (POLICY CMP0069)
if (POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif (POLICY CMP0072)

project( sdf )

set( CMAKE_CXX_STACK_SIZE "10000000" )
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON)
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

set( INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" )

option( SDF_USE_SYSTEM_EIGEN
    "Use system Eigen. If false, please set EIGEN_INCLUDE_DIR manually" ON )

if ( CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang") )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations -O3 -funroll-loops -g" )
elseif( MSVC )
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /GLT /Ox")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /GLT")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ( )

file(GLOB SDF_SOURCES ${SRC_DIR}/*.cpp)
set(SDF_PUBLIC_HEADERS ${INCLUDE_DIR}/sdf/sdf.hpp)

set( DEPENDENCIES )

find_package(Threads REQUIRED)

if (SDF_USE_SYSTEM_EIGEN)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)
    message(STATUS "Using system Eigen")
    set(DEPENDENCIES ${DEPENDENCIES} Eigen3::Eigen)
else()
    # Set EIGEN_INCLUDE_DIR manually
    include_directories(${EIGEN_INCLUDE_DIR})
endif()

add_library( sdf STATIC ${SDF_SOURCES} )

target_include_directories(
    sdf PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries( sdf PUBLIC
    ${DEPENDENCIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)