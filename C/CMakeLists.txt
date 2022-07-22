cmake_minimum_required(VERSION 3.10)

# specify the C++ standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

# set the project name and version
project(cProject VERSION 1.0)

include_directories(cTest PUBLIC
  ${cProject_SOURCE_DIR}/inc
  ${cProject_SOURCE_DIR}/src
  ${cProject_SOURCE_DIR}/test
  )

file(GLOB all_SRCS
  "${cProject_SOURCE_DIR}/inc/*.h"
  "${cProject_SOURCE_DIR}/src/*.c"
  "${cProject_SOURCE_DIR}/test/*.c"
  )

add_executable(cTest ${all_SRCS})
