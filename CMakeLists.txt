# Copyright (C) Point One Navigation - All Rights Reserved
cmake_minimum_required(VERSION 3.14)

set(LIBOSR_PRODUCER_URL "https://s3.amazonaws.com/files.pointonenav.com/polaris/ssr/osr_producer.v0.2.2.tar.gz" CACHE STRING "Specify the URL of the verison of libosr_producer to be used.")
set(LIBOSR_PRODUCER_HASH "a1c7f7ed0de2f1799d7d6e2be00c4795" CACHE STRING "Specify the MD5 hash libosr_producer zip file to be downloaded.")

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
if ("${LIBOSR_PRODUCER_URL}" STREQUAL "")
  # TODO Update this URL when we figure out where the file lives.
  set(LIBOSR_PRODUCER_URL ${CMAKE_CURRENT_SOURCE_DIR}/libosr_producer.zip)
  # TODO Set the correct MD5 hash for the file.
  #set(LIBOSR_PRODUCER_HASH "abcd1234")
  set(LIBOSR_PRODUCER_HASH "")
endif()

if (IS_DIRECTORY "${LIBOSR_PRODUCER_URL}")
  get_filename_component(LIBOSR_PRODUCER_URL "${LIBOSR_PRODUCER_URL}"
                         REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
  message(STATUS "Using libosr_producer: ${LIBOSR_PRODUCER_URL}")
  add_subdirectory(${LIBOSR_PRODUCER_URL})
else()
  # Prepend MD5= to the hash if specified.
  if (NOT "${LIBOSR_PRODUCER_HASH}" STREQUAL "")
      set(LIBOSR_PRODUCER_HASH "MD5=${LIBOSR_PRODUCER_HASH}")
  endif()

  message(STATUS "Using libosr_producer: ${LIBOSR_PRODUCER_URL}")
  FetchContent_Declare(
      libosr_producer
      URL ${LIBOSR_PRODUCER_URL}
      URL_HASH ${LIBOSR_PRODUCER_HASH}
  )
  FetchContent_MakeAvailable(libosr_producer)
endif()

# Import the Point One Polaris client.
ExternalProject_Add(
    polaris
    URL https://github.com/PointOneNav/polaris/archive/b57b7a986a117a6551e2d722d08a868a7a50f971.zip
    URL_HASH MD5=4592480063abab7d36fb7de85cc8f312
    PREFIX ${CMAKE_BINARY_DIR}/external
    INSTALL_COMMAND ""
    BUILD_COMMAND $(MAKE) polaris_cpp_client
)
ExternalProject_Get_Property(polaris BINARY_DIR)
ExternalProject_Get_Property(polaris SOURCE_DIR)
add_library(libpolaris_cpp_client SHARED IMPORTED GLOBAL)
add_dependencies(libpolaris_cpp_client polaris)
set_target_properties(
    libpolaris_cpp_client
    PROPERTIES
    #INTERFACE_INCLUDE_DIRECTORIES "${SOURCE_DIR}/src;${SOURCE_DIR}/c/src"
    INTERFACE_LINK_DEPENDS ${GLOG_LIBRARIES}
    IMPORTED_LOCATION ${BINARY_DIR}/libpolaris_cpp_client.so
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
)

# Note: ExternalProject does not download the source code until build time, so
# ${SOURCE_DIR} (i.e., external/polaris) is empty at configure time and we
# cannot set INTERFACE_INCLUDE_DIRECTOIRES above or cmake will fail saying that
# ${SOURCE_DIR}/src doesn't exist.
#
# In an ideal world, ExternalProject would download the code at configure time,
# and the property would work as expected. Then, wherever we use this library
# we could simply reference it with target_link_libraries(), and it would
# automatically add the extra include paths. Because of the download order, we
# can't do that unfortunately.
#
# The easiest workaround is to define this INCLUDE_DIRS variable, and then
# explicitly pass it to target_include_directories() wherever we use this
# library. Annoying, but not the end of the world.
set(libpolaris_cpp_client_INCLUDE_DIRS "${SOURCE_DIR}/src;${SOURCE_DIR}/c/src")

################################################################################
# Example Applications
################################################################################

add_subdirectory(examples/septentrio_osr_example)
