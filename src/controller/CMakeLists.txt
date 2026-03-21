add_library(controller_lib Controller.cpp AgentTcpSession.cpp StateStore.cpp)
target_link_libraries(controller_lib PUBLIC core_lib)

add_executable(controller main.cpp)
target_link_libraries(controller PRIVATE controller_lib)
