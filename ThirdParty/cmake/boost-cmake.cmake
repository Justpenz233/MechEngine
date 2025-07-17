include(FetchContent)
cmake_policy(SET CMP0135 NEW)

set(BOOST_INCLUDE_LIBRARIES serialization system foreach any variant graph property_map multiprecision heap filesystem safe_numerics)
set(BOOST_RUNTIME_LINK shared)
set(BUILD_SHARED_LIBS ON)
FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.tar.gz
)
FetchContent_MakeAvailable(Boost)

add_library(Boost::boost INTERFACE IMPORTED)
foreach(lib ${BOOST_INCLUDE_LIBRARIES})
    target_link_libraries(Boost::boost INTERFACE Boost::${lib})

    get_target_property(target_type boost_${lib} TYPE)
    if (target_type STREQUAL "INTERFACE_LIBRARY")
        if (MSVC)
            target_compile_options(boost_${lib} INTERFACE -W0)
        else ()
            target_compile_options(boost_${lib} INTERFACE -w)
        endif ()
    elseif (NOT target_type STREQUAL "INTERFACE_LIBRARY")
        if (MSVC)
            target_compile_options(boost_${lib} PRIVATE -W0)
        else ()
            target_compile_options(boost_${lib} PRIVATE -w)
        endif ()
    endif()

#    if (MSVC)
#        target_compile_options(boost_${lib} PRIVATE -W0)
#    else ()
#        target_compile_options(boost_${lib} PRIVATE -w -Wno-error -Wno-old-style-cast)
#    endif ()
endforeach()