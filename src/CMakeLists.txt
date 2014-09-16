add_library(task_distribution
  computing_unit.cpp
  computing_unit_manager.cpp
  key.cpp
  runnable.cpp
  task_manager.cpp
)

add_library(task_distribution_mpi
  computing_unit_manager_mpi.cpp
  task_manager_mpi.cpp
)

target_link_libraries(task_distribution
  ${Boost_LIBRARIES}
)

target_link_libraries(task_distribution_mpi
  task_distribution
  mpi_handler
  ${MPI_LIB}
)

add_executable(main
  main.cpp
)

target_link_libraries(main
  task_distribution_mpi
)
