# Copyright (C) Point One Navigation - All Rights Reserved
project(osr-producer)

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
include_directories(${EXTERNAL_INSTALL_LOCATION}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib
                 ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_library(GLOG glog)
find_library(GFLAGS gflags)

execute_process(
    COMMAND /usr/bin/uname -m
    OUTPUT_VARIABLE ARCH
)
string(STRIP ${ARCH} ARCH)
ExternalProject_Add(
    osr-producer
    # TODO: Update this URL when we figure out where the file lives.
    URL "http://localhost:8000/osr_producer.v0.2.1.tar.gz"
    # URL "https://s3.amazonaws.com/files.pointonenav.com/osr_producer.v0.2.1.tar.gz"
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/external
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

ExternalProject_Add(
    polaris
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/external
    GIT_REPOSITORY https://github.com/PointOneNav/polaris.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

################################################################################
# Example Applications
################################################################################

add_subdirectory(examples/septentrio)
