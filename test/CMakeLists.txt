if(NOT NO_MPI)
  add_custom_target(test COMMAND run_tests.bin
                         COMMAND mpirun -np 2 run_tests_mpi.bin
                         DEPENDS run_tests.bin run_tests_mpi.bin)
else()
  add_custom_target(test COMMAND run_tests.bin
                         DEPENDS run_tests.bin)
endif()

add_executable(run_tests.bin EXCLUDE_FROM_ALL
  #computing_unit.cpp
  task_manager.cpp
)

target_link_libraries(run_tests.bin gtest gtest_main
  task_distribution
)

if(NOT NO_MPI)
  add_executable(run_tests_mpi.bin EXCLUDE_FROM_ALL
    #computing_unit_mpi.cpp
    task_manager_mpi.cpp
    test_mpi_main.cpp
  )

  target_link_libraries(run_tests_mpi.bin gtest
    task_distribution
  )
endif()
