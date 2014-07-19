add_executable(run_tests.bin EXCLUDE_FROM_ALL
  computing_unit.cpp
)

target_link_libraries(run_tests.bin gtest gtest_main
  task_distribution
)

add_custom_target(test COMMAND run_tests.bin)
add_dependencies(test run_tests.bin)
