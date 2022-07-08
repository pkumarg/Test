cmake_minimum_required(VERSION 3.10)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name and version
project(cppProject VERSION 1.0)

include_directories(cppTest PUBLIC
  ${cppProject_SOURCE_DIR}/inc
  ${cppProject_SOURCE_DIR}/src
  ${cppProject_SOURCE_DIR}/test
  )

file(GLOB all_SRCS
  "${cppProject_SOURCE_DIR}/inc/*.h"
  "${cppProject_SOURCE_DIR}/src/*.cpp"
  "${cppProject_SOURCE_DIR}/test/*.cpp"
  )

add_executable(cppTest ${all_SRCS})
