cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_COMPILER "clang++")
project(pixelsim)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11")

set(SRCS main.cc ship.cc shaders.cc)
add_executable(${CMAKE_PROJECT_NAME} ${SRCS})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

find_library(OPENGL_LIBRARY OpenGL)

find_package(PNG REQUIRED)

include_directories(${GLFW_INCLUDE_DIRS} ${PNG_INCLUDE_DIR})
target_link_libraries(${CMAKE_PROJECT_NAME}
                      ${GLFW_LIBRARIES}
                      ${OPENGL_LIBRARY}
                      ${PNG_LIBRARY})
