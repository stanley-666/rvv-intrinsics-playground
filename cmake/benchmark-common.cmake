function(rvv_add_benchmark benchmark_name benchmark_source)
  add_executable(${benchmark_name} "${benchmark_source}")
  target_link_libraries(${benchmark_name} PRIVATE m)
  target_compile_options(${benchmark_name} PRIVATE ${RVV_COMMON_FLAGS})
  target_compile_definitions(${benchmark_name} PRIVATE BENCHMARK_REPEAT=${BENCHMARK_REPEAT})
  if(CMAKE_SOURCE_DIR STREQUAL RVV_PLAYGROUND_ROOT_DIR)
    set_target_properties(${benchmark_name} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks/${benchmark_name}"
    )
  endif()
  if(RVV_LINK_STATIC)
    target_link_options(${benchmark_name} PRIVATE -static)
  endif()
endfunction()
