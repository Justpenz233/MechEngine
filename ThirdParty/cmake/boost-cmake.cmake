include(FetchContent)
cmake_policy(SET CMP0135 NEW)

set(BOOST_INCLUDE_LIBRARIES system foreach any variant graph property_map multiprecision heap)
FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.81.0/boost-1.81.0.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(Boost)