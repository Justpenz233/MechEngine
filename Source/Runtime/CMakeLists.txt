cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(ENGINE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${MECHENGINE_ROOT_DIR}/bin")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rd")
set(BINARY_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/")
list(PREPEND CMAKE_MODULE_PATH ${THIRD_PARTY_DIR})
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE INTERNAL "" FORCE)

add_definitions(-w)
add_compile_definitions(_USE_MATH_DEFINES)

if(MSVC)
    add_compile_options(/MP)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif()

project(MechEngineRuntime)
file(GLOB_RECURSE ENGINE_SOURCE_FILES
        *.cpp *.ipp
)

file(GLOB_RECURSE ENGINE_HEADER_FILES
        *.h
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_HEADER_FILES} ${ENGINE_SOURCE_FILES})

add_library(MechEngineRuntime SHARED ${ENGINE_SOURCE_FILES} ${ENGINE_HEADER_FILES})
target_compile_features(MechEngineRuntime PUBLIC cxx_std_20)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    # Set debug-specific options
    set(SANITIZE_FLAGS "-fsanitize=undefined,return")

    target_compile_options(MechEngineRuntime PUBLIC
            $<$<CONFIG:Debug>:${SANITIZE_FLAGS}>
    )

    # Apply link options publicly as well
    target_link_options(MechEngineRuntime PUBLIC
            $<$<CONFIG:Debug>:${SANITIZE_FLAGS}>
    )
endif()

set_target_properties(MechEngineRuntime PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(MechEngineRuntime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(MechEngineRuntime PUBLIC "UI")
target_include_directories(MechEngineRuntime PUBLIC "UI/Imgui")
target_include_directories(MechEngineRuntime PUBLIC "Core")
target_include_directories(MechEngineRuntime PUBLIC "Classes")


target_link_libraries(MechEngineRuntime PUBLIC ThirdParty)
## Reflection
set_target_properties(MechEngineRuntime PROPERTIES PARSE_HEADERS "${ENGINE_HEADER_FILES}")

add_custom_command(
        TARGET MechEngineRuntime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/bin
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Copying my_executable to runtime directory"
)