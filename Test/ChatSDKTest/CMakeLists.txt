cmake_minimum_required(VERSION 3.10)

project(ChatSDKTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_executable(ChatSDKTest
    ChatSDKTest.cc
    ../../ChatSDK/Src/Util/LogModule.cc
    ../../ChatSDK/Src/DataManager.cc
    ../../ChatSDK/Src/SessionManager.cc
    ../../ChatSDK/Src/ChatSDK.cc
    ../../ChatSDK/Src/LLMManager.cc
    ../../ChatSDK/Src/DeepSeekProvider.cc
    ../../ChatSDK/Src/ChatGPTProvider.cc
    ../../ChatSDK/Src/KimiProvider.cc
    ../../ChatSDK/Src/OllamaProvider.cc
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# 项目头文件
include_directories(../../ChatSDK/Include)

target_compile_definitions(ChatSDKTest
    PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
)

target_link_libraries(ChatSDKTest
    jsoncpp
    fmt
    spdlog
    gtest
    sqlite3
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
