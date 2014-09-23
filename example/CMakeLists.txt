add_executable(example.bin
  example.cpp
)

target_link_libraries(example.bin
  task_distribution_mpi
)
