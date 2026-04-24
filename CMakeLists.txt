cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_STANDARD   17)
set(CMAKE_CXX_STANDARD 20)

include(lua550.cmake)

project(Luna)
add_library(Luna INTERFACE)
target_link_libraries(Luna INTERFACE lua550)
target_include_directories(Luna INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_executable(demo demo.cpp)
target_link_libraries(demo Luna)
