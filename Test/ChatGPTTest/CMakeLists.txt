cmake_minimum_required(VERSION 3.10)

project(TestDeepSeek)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_executable(TestChatGpt
    ChatGptTestSend.cc  
    ../../ChatSDK/Src/Util/LogModule.cc
    ../../ChatSDK/Src/ChatGPTProvider.cc
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# 项目头文件
include_directories(../../ChatSDK/Include)

target_compile_definitions(TestChatGpt
    PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
)

target_link_libraries(TestChatGpt
    jsoncpp
    fmt
    spdlog
    gtest
    sqlite3
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)