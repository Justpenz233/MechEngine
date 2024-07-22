file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.h)
add_library(bvh STATIC ${SOURCES} ${HEADERS})

find_package(Threads)
if (Threads_FOUND)
    # Link with the threading library of the system, which may
    # be required by standard header <thread> on some systems
    target_link_libraries(bvh PRIVATE Threads::Threads)
endif()

target_include_directories(bvh PUBLIC ../..)
set_target_properties(bvh PROPERTIES CXX_STANDARD 20)
if (MSVC)
    target_compile_options(bvh PRIVATE /fp:precise)
endif ()