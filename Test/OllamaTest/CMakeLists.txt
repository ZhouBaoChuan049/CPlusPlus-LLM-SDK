cmake_minimum_required(VERSION 3.10)

project(TestOllama)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_executable(TestOllama
    OllamaSendTest.cc  
    ../../ChatSDK/Src/Util/LogModule.cc
    ../../ChatSDK/Src/OllamaProvider.cc
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# 项目头文件
include_directories(../../ChatSDK/Include)

target_compile_definitions(TestOllama
    PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
)

target_link_libraries(TestOllama
    jsoncpp
    fmt
    spdlog
    gtest
    sqlite3
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)