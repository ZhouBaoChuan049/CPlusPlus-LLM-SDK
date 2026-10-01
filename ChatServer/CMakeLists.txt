# 设置Cmake的最小版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(AIChatServer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型Debug
set(CMAKE_BUILD_TYPE Debug)

# 添加可执行文件
add_executable(AIChatServer main.cpp ChatServer.cc)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# 查找依赖库
find_package(OpenSSL REQUIRED)

# 添加CPPHTTPLIB_OPENSSL_SUPPORT定义
target_compile_definitions(AIChatServer PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# 设置静态库的目录
link_directories(/usr/local/lib)

# 链接库
target_link_libraries(
    AIChatServer 
    libChatSDK
    jsoncpp 
    fmt  
    spdlog 
    gflags
    OpenSSL::SSL 
    OpenSSL::Crypto 
    sqlite3
    gflags
)