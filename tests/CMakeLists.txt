# Include directory for tests
include_directories(include)

include(GoogleTest)

# Command Retry Test
add_executable(command_retry_test src/CommandRetryTest.cpp src/test_utils/MockFaultyComm.cpp)
target_link_libraries(command_retry_test PRIVATE controller_lib core_lib GTest::gtest_main spdlog::spdlog)

gtest_discover_tests(command_retry_test)

# Integration Test (GTest)
add_executable(integration_test src/IntegrationTest.cpp ../src/agent/Agent.cpp)
target_link_libraries(integration_test PRIVATE controller_lib core_lib GTest::gtest_main spdlog::spdlog)

gtest_discover_tests(integration_test)

# Duplicate Message Test (GTest)
add_executable(duplicate_message_test src/DuplicateMessageTest.cpp ../src/agent/Agent.cpp)
target_link_libraries(duplicate_message_test PRIVATE controller_lib core_lib GTest::gtest_main spdlog::spdlog)
gtest_discover_tests(duplicate_message_test)

# Message Parser / CRC Validation Test (GTest)
add_executable(message_parser_test src/MessageParserTest.cpp)
target_link_libraries(message_parser_test PRIVATE core_lib GTest::gtest_main spdlog::spdlog)
gtest_discover_tests(message_parser_test)
