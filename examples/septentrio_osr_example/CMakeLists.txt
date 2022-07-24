# Septentrio example application (see septentrio_main.cc for details).
add_executable(septentrio_osr_example septentrio_main.cc serial_port.cc)

target_include_directories(septentrio_osr_example PUBLIC ${libpolaris_cpp_client_INCLUDE_DIRS})
target_link_libraries(septentrio_osr_example libpolaris_cpp_client)

target_link_libraries(septentrio_osr_example libosr_producer)

target_include_directories(septentrio_osr_example PUBLIC ${GFLAGS_INCLUDE_DIRS})
target_link_libraries(septentrio_osr_example ${GFLAGS_LIBRARIES})

target_include_directories(septentrio_osr_example PUBLIC ${GLOG_INCLUDE_DIRS})
target_link_libraries(septentrio_osr_example ${GLOG_LIBRARIES})
