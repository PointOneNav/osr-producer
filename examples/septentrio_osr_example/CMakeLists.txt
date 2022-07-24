# Septentrio sample application

add_executable(septentrio_osr_example septentrio_main.cc serial_port.cc)
add_dependencies(septentrio_osr_example osr-producer)
# We're expecting CMAKE_SYSTEM_PROCESSOR to be either "x86_64" or "aarch64".
target_link_libraries(septentrio_osr_example "libosr_producer.${CMAKE_SYSTEM_PROCESSOR}.a")
target_link_libraries(septentrio_osr_example libpolaris_cpp_client)

target_include_directories(septentrio_osr_example PUBLIC ${GFLAGS_INCLUDE_DIRS})
target_link_libraries(septentrio_osr_example ${GFLAGS_LIBRARIES})

target_include_directories(septentrio_osr_example PUBLIC ${GLOG_INCLUDE_DIRS})
target_link_libraries(septentrio_osr_example ${GLOG_LIBRARIES})
