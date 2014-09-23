add_library(task_distribution SHARED
  computing_unit.cpp
  computing_unit_manager.cpp
  key.cpp
  runnable.cpp
  task_manager.cpp
)

add_library(task_distribution_mpi SHARED
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
