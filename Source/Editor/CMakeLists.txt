project(MechEngineEditor)

set(CMAKE_CXX_STANDARD 20)

set(EDITOR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE EDITOR_SOURCE_FILES
        *.cpp *.ipp
        )

file(GLOB_RECURSE EDITOR_HEADER_FILES
    *.h
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${EDITOR_HEADER_FILES} ${EDITOR_SOURCE_FILES})

add_library(MechEngineEditor SHARED ${EDITOR_SOURCE_FILES} ${EDITOR_HEADER_FILES})
target_compile_definitions(MechEngineEditor PRIVATE EDITOR_API_EXPORT)
target_include_directories(MechEngineEditor PUBLIC ${EDITOR_SOURCE_DIR})
target_include_directories(MechEngineEditor PUBLIC ${EDITOR_SOURCE_DIR}/Core)
if(MSVC)
        set_target_properties(MechEngineEditor PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_link_libraries(MechEngineEditor PUBLIC MechEngineRuntime)
target_link_libraries(MechEngineEditor PUBLIC ${USER_PROJECT_NAME})

set(PreCompileTargets MechEngineRuntime ${USER_PROJECT_NAME})
PreCompile(${CMAKE_CURRENT_SOURCE_DIR} "${PreCompileTargets}")