add_library(core_lib STATIC 
    message/Message.cpp 
    message/BinaryMessageParser.cpp 
    message/payload/HelloPayload.cpp 
    message/payload/HeartbeatPayload.cpp 
    message/payload/StatePayload.cpp 
    message/payload/CmdSetModePayload.cpp 
    message/payload/AckPayload.cpp 
    message/payload/NackPayload.cpp 
    comm/TcpComm.cpp)
    
target_include_directories(core_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(core_lib PUBLIC asio_lib spdlog::spdlog nlohmann_json::nlohmann_json)
