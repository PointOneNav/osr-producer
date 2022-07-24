# Copyright (C) Point One Navigation - All Rights Reserved
cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

project(p1_osr_producer)

# Set compilation flags.
add_compile_options(-Wall -Werror)

################################################################################
# Library Definitions
################################################################################

include(ExternalProject)
include(FetchContent)

# Locate 3rd party dependencies.
option(GLOG_PREFER_EXPORTED_GLOG_CMAKE_CONFIGURATION TRUE)
find_package(Glog REQUIRED)
option(GFLAGS_PREFER_EXPORTED_GFLAGS_CMAKE_CONFIGURATION TRUE)
find_package(Gflags REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# Import the Point One OSRProducer library (libosr_producer.a).
FetchContent_Declare(
    libosr_producer
    # TODO: Update this URL when we figure out where the file lives.
    URL ${CMAKE_SOURCE_DIR}/libosr_producer.zip
    # TODO: URL_HASH xxx
)
FetchContent_MakeAvailable(libosr_producer)

# Import the Point One Polaris client.
ExternalProject_Add(
    polaris
    URL https://github.com/PointOneNav/polaris/archive/b57b7a986a117a6551e2d722d08a868a7a50f971.zip
    URL_HASH MD5=4592480063abab7d36fb7de85cc8f312
    PREFIX ${CMAKE_BINARY_DIR}/external
    INSTALL_COMMAND ""
)
ExternalProject_Get_Property(polaris BINARY_DIR)
ExternalProject_Get_Property(polaris SOURCE_DIR)
add_library(libpolaris_cpp_client SHARED IMPORTED GLOBAL)
add_dependencies(libpolaris_cpp_client polaris)
set_target_properties(
    libpolaris_cpp_client
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SOURCE_DIR}/src;${SOURCE_DIR}/c/src"
    INTERFACE_LINK_DEPENDS ${GLOG_LIBRARIES}
    IMPORTED_LOCATION ${BINARY_DIR}/libpolaris_cpp_client.so
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
)

################################################################################
# Example Applications
################################################################################

add_subdirectory(examples/septentrio_osr_example)
