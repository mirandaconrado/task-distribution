add_library(task_distribution
  computing_unit.cpp
  computing_unit_manager.cpp
  task_manager.cpp
)

target_link_libraries(task_distribution
  mpi_handler
  ${Boost_LIBRARIES}
  ${MPI_LIB}
)

add_executable(main
  main.cpp
)

target_link_libraries(main
  task_distribution
)
