add_library(task_distribution
  computing_unit.cpp
  computing_unit_manager.cpp
)

target_link_libraries(task_distribution
  ${Boost_LIBRARIES}
  ${MPI_LIB}
)
