include(FetchContent)
FetchContent_Declare(
        LuisaCompute
        GIT_REPOSITORY    https://github.com/LuisaGroup/LuisaCompute.git
        GIT_TAG           stable
)

FETCHCONTENT_MAKEAVAILABLE(LuisaCompute)