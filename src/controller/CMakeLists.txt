add_executable(controller main.cpp Controller.cpp AgentTcpSession.cpp StateStore.cpp)
target_link_libraries(controller PRIVATE core_lib)
