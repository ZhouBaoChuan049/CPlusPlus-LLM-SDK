cmake_minimum_required(VERSION 3.10)

project(ChatServerTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_executable(ChatServerTest
    ChatServerTest.cc
    ../../ChatServer/ChatServer.cc
    ../../Src/Util/LogModule.cc
    ../../Src/DataManager.cc
    ../../Src/SessionManager.cc
    ../../Src/ChatSDK.cc
    ../../Src/LLMManager.cc
    ../../Src/DeepSeekProvider.cc
    ../../Src/ChatGPTProvider.cc
    ../../Src/KimiProvider.cc
    ../../Src/OllamaProvider.cc
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# 项目头文件：SDK 头文件 + ChatServer 头文件
include_directories(../../Include ../../ChatServer)

target_compile_definitions(ChatServerTest
    PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
)

target_link_libraries(ChatServerTest
    jsoncpp
    fmt
    spdlog
    gtest
    sqlite3
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)