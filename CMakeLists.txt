cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_COMPILER "clang++-3.3")
project(pixelFEM)

set(LLVM_DIR /usr/local/lib/llvm-3.3)

# -U__STRICT_ANSI__ is a workaround for a bug in 10.6's math.h
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -U__STRICT_ANSI__ -g -std=c++11 -stdlib=libc++ -I${LLVM_DIR}/lib/c++/v1")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${LLVM_DIR}/usr/lib")

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
