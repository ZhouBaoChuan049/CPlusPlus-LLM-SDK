cmake_minimum_required(VERSION 3.10)

project(ChatSDKTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_executable(ChatSDKTest
    ChatSDKTest.cc
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

# 项目头文件
include_directories(../../Include)

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
