set (ThirdPartyDir ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    add_compile_options(/W0)  # Suppress all warnings
endif()

# Use system packages if available
find_package(Boost QUIET)
find_package(Eigen QUIET)

if (NOT TARGET luisa::compute)
    option(EASTL_ENABLE_MIMALLOC "Enable mimalloc" OFF)
    option(LUISA_COMPUTE_ENABLE_DSL "Enable C++ DSL" ON)
    option(LUISA_COMPUTE_ENABLE_TENSOR "Enable C++ DSL tensor extension" OFF)
    option(LUISA_COMPUTE_ENABLE_DX "Enable DirectX backend" ON)
    option(LUISA_COMPUTE_ENABLE_METAL "Enable Metal backend" ON)
    option(LUISA_COMPUTE_ENABLE_CUDA "Enable CUDA backend" ON)
    option(LUISA_COMPUTE_ENABLE_CUDA_EXT_LCUB "Enable CUDA extension: LCUB" OFF)
    option(LUISA_COMPUTE_ENABLE_VULKAN "Enable Vulkan backend" ON)
    option(LUISA_COMPUTE_ENABLE_CPU "Enable CPU backend" ON)
    option(LUISA_COMPUTE_ENABLE_REMOTE "Enable Remote backend" OFF)
    option(LUISA_COMPUTE_ENABLE_GUI "Enable GUI support" ON)
    set(LUISA_COMPUTE_BUILD_TESTS OFF)
    set(LUISA_COMPUTE_ENABLE_RUST OFF)
    add_subdirectory(luisa-compute)
endif ()

if (NOT TARGET json11)
    set(JSON11_BUILD_TESTS FALSE)
    add_subdirectory(json11)
endif()

if (NOT TARGET libigl)
    option(LIBIGL_USE_STATIC_LIBRARY "Use libigl as static library" OFF)
    option(LIBIGL_INSTALL "Enable installation of libigl targets" OFF)
    option(LIBIGL_IMGUI "Build target igl::imgui" OFF)     # use external imgui
    option(LIBIGL_GLFW "Build target igl::glfw" OFF)
    option(LIBIGL_COPYLEFT_CGAL "Build target igl_copyleft::cgal" ON) # use CGAL for mesh boolean
    option(LIBIGL_RESTRICTED_TRIANGLE "Build target igl_restricted::triangle" ON) # for SCAF parametaerization
    add_subdirectory(libigl)
endif()

if (NOT TARGET bvh)
    add_subdirectory(bvh)
endif ()

if(NOT TARGET tinysplinecxx)
    add_subdirectory(tinyspline)
endif ()

if (NOT TARGET pmp)
    add_subdirectory(libpmp)
endif ()

add_library(ThirdParty INTERFACE)
target_link_libraries(ThirdParty INTERFACE
        json11
        igl_copyleft::cgal
        igl_restricted::triangle
        spdlog::spdlog_header_only
        tinysplinecxx
        magic_enum
        luisa::compute
        bvh
        pmp
)