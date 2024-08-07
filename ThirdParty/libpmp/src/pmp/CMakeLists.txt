file(GLOB SOURCES ./*.cpp)
file(GLOB HEADERS ./*.h)

if(EMSCRIPTEN)
  add_library(pmp STATIC ${SOURCES} ${HEADERS})
else()
  add_library(pmp ${SOURCES} ${HEADERS})
  set_property(TARGET pmp PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
  target_link_libraries(pmp PUBLIC OpenMP::OpenMP_CXX)
endif()

if (TARGET Eigen3::Eigen)
  target_link_libraries(pmp PUBLIC Eigen3::Eigen)
else()
  find_package(Eigen3 REQUIRED)
  target_link_libraries(pmp PUBLIC Eigen3::Eigen)
endif ()

set_target_properties(pmp PROPERTIES VERSION ${PROJECT_VERSION})

if(WITH_CLANG_TIDY)
  set_target_properties(pmp PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

if(NOT EMSCRIPTEN AND PMP_INSTALL)

  target_include_directories(
    pmp
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
           $<INSTALL_INTERFACE:include/>)

  target_compile_features(pmp PUBLIC cxx_std_17)

  install(
    TARGETS pmp
    EXPORT pmpTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES
    DESTINATION include)

  install(FILES ${HEADERS} DESTINATION include/pmp/)
else()
  target_include_directories(pmp PUBLIC
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../)
endif()

if(PMP_BUILD_VIS)
  add_subdirectory(visualization)
endif()

include(algorithms/CMakeLists.txt)