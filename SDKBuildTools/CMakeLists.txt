cmake_minimum_required(VERSION 3.10)

project(ChatSDK CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# SDK 源文件
set(SDK_SOURCES
    ../ChatSDK/Src/ChatSDK.cc
    ../ChatSDK/Src/SessionManager.cc
    ../ChatSDK/Src/DataManager.cc
    ../ChatSDK/Src/UserManager.cc
    ../ChatSDK/Src/LLMManager.cc
    ../ChatSDK/Src/DeepSeekProvider.cc
    ../ChatSDK/Src/ChatGPTProvider.cc
    ../ChatSDK/Src/KimiProvider.cc
    ../ChatSDK/Src/OllamaProvider.cc
    ../ChatSDK/Src/Util/LogModule.cc
)

add_library(ChatSDK STATIC ${SDK_SOURCES})

target_include_directories(ChatSDK PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../ChatSDK/Include
)

# httplib 需要启用 OpenSSL 支持
target_compile_definitions(ChatSDK PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# 静态库 libChatSDK.a 输出到 BuildTools 目录下
set_target_properties(ChatSDK PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(ChatSDK
    jsoncpp
    fmt
    spdlog
    sqlite3
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
