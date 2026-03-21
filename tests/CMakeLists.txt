# Include directory for tests
include_directories(include)

# Command Retry Test
add_executable(command_retry_test src/CommandRetryTest.cpp src/test_utils/MockFaultyComm.cpp)
target_link_libraries(command_retry_test PRIVATE controller_lib core_lib spdlog::spdlog)

# Device Simulator Test (GTest)
add_executable(device_simulator_test src/device_simulator_test.cpp)
target_link_libraries(device_simulator_test PRIVATE core_lib GTest::gtest_main spdlog::spdlog)

include(GoogleTest)
gtest_discover_tests(device_simulator_test)
