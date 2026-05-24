cmake_minimum_required(VERSION 3.16)

project(PikoRL VERSION 0.1.0 LANGUAGES C CXX)

option(PICORL_BUILD_DOCS "Build Doxygen documentation" ON)

add_subdirectory(qamrpp-lpikorl)

if(PICORL_BUILD_DOCS)
  add_subdirectory(docs)
endif()
