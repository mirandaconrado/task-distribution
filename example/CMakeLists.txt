add_executable(example.bin
  example.cpp
)

if (ENABLE_MPI)
  target_link_libraries(example.bin
    task_distribution_mpi
  )
else()
  target_link_libraries(example.bin
    task_distribution
  )
endif()
