project(tinysplinecxx)

set(CMAKE_CXX_STANDARD 11)
add_library(tinysplinecxx SHARED tinysplinecxx.cxx tinyspline.c parson.c)
target_include_directories(tinysplinecxx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})