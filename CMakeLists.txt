# Copyright (C) Point One Navigation - All Rights Reserved
cmake_minimum_required(VERSION 3.3.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(p1_osr_producer)

# Set compilation flags.
add_compile_options(-Wall -Werror)

################################################################################
# Library Definitions
################################################################################

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
include_directories(${EXTERNAL_INSTALL_LOCATION}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib
                 ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(Glog REQUIRED)
find_package(Gflags REQUIRED)

ExternalProject_Add(
    osr-producer
    # TODO: Update this URL when we figure out where the file lives.
    URL "http://localhost:8000/osr_producer.v0.2.1.tar.gz"
    # URL "https://s3.amazonaws.com/files.pointonenav.com/osr_producer.v0.2.1.tar.gz"
    # TODO: URL_HASH xxx
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/external
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

ExternalProject_Add(
    polaris
    URL https://github.com/PointOneNav/polaris/archive/b57b7a986a117a6551e2d722d08a868a7a50f971.zip
    URL_HASH MD5=4592480063abab7d36fb7de85cc8f312
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/external
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

################################################################################
# Example Applications
################################################################################

add_subdirectory(examples/septentrio_osr_example)
