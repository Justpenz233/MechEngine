#include(ExternalProject)
#
#set(LuisaRoot ${ThirdPartyDir}/luisa-compute)
#
#ExternalProject_Add(LuisaCompute
#        PREFIX            ${LuisaRoot}
#        GIT_REPOSITORY    https://github.com/LuisaGroup/LuisaCompute.git
#        GIT_TAG           stable
#        CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=${LuisaRoot}
#)
#
#find_package(Threads REQUIRED)
#include(${LuisaRoot}/lib/cmake/LuisaCompute/LuisaComputeTargets.cmake)
#add_library(LuisaComputeLib INTERFACE)
#target_link_libraries(LuisaComputeLib INTERFACE luisa::compute)


include(FetchContent)
FetchContent_Declare(
        LuisaCompute
        GIT_REPOSITORY    https://github.com/LuisaGroup/LuisaCompute.git
        GIT_TAG           stable
)

FETCHCONTENT_MAKEAVAILABLE(LuisaCompute)