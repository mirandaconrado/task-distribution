if(ENABLE_MPI)
  add_custom_target(test COMMAND mpirun -np 2 run_tests_mpi.bin
                         DEPENDS run_tests_mpi.bin)

  add_executable(run_tests_mpi.bin EXCLUDE_FROM_ALL
    computing_unit_mpi.cpp
    computing_unit_manager_mpi.cpp
    test_mpi_main.cpp
  )

  target_link_libraries(run_tests_mpi.bin gtest
    task_distribution
  )
else()
  add_custom_target(test COMMAND run_tests.bin
                         DEPENDS run_tests.bin)

  add_executable(run_tests.bin EXCLUDE_FROM_ALL
    computing_unit.cpp
    computing_unit_manager.cpp
  )

  target_link_libraries(run_tests.bin gtest gtest_main
    task_distribution
  )
endif()
