# Septentrio sample application

add_executable(septentrio septentrio_main.cc serial_port.cc)
add_dependencies(septentrio polaris)
add_dependencies(septentrio osr-producer)
target_link_libraries(septentrio "libosr_producer.${ARCH}.a")
target_link_libraries(septentrio "${CMAKE_SOURCE_DIR}/external/src/polaris-build/libpolaris_cpp_client.so")
target_link_libraries(septentrio "${GLOG}")
target_link_libraries(septentrio "${GFLAGS}")
