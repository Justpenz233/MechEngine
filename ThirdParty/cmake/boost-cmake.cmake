include(FetchContent)
set(BOOST_INCLUDE_LIBRARIES system foreach any variant graph property_map multiprecision heap)
FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.81.0/boost-1.81.0.tar.xz
)
FetchContent_MakeAvailable(Boost)