# Copyright (C) Point One Navigation - All Rights Reserved
project(osrproducer)

cmake_minimum_required(VERSION 3.3.2)

# Set compilation flags.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Werror)

################################################################################
# Library Definitions
################################################################################

include(ExternalProject)
find_package(Git REQUIRED)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
include_directories(${EXTERNAL_INSTALL_LOCATION}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_library(GLOG glog)
find_library(GFLAGS gflags)

ExternalProject_Add(
    polaris_project
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/external
    GIT_REPOSITORY https://github.com/PointOneNav/polaris.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)
find_library(POLARIS polaris_cpp_client ${CMAKE_SOURCE_DIR}/external/src/polaris_project-build)

add_library(osr_producer INTERFACE)

################################################################################
# Example Applications
################################################################################

add_subdirectory(examples/septentrio)
