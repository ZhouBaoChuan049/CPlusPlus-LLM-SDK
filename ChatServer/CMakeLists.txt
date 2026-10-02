# 设置Cmake的最小版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(AIChatServer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型Debug
set(CMAKE_BUILD_TYPE Debug)

# 设置静态库的目录（须在 add_executable 之前，link_directories 只对后创建的目标生效）
link_directories(/usr/local/ChatSDK/lib)

# 添加可执行文件
add_executable(AIChatServer Main.cc ChatServer.cc)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# 查找依赖库
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# 添加CPPHTTPLIB_OPENSSL_SUPPORT定义
target_compile_definitions(AIChatServer PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# ChatSDK 安装位置：头文件与静态库均从该目录查找
target_include_directories(AIChatServer PRIVATE /usr/local/ChatSDK/include)

# 链接库
target_link_libraries(
    AIChatServer 
    ChatSDK
    jsoncpp 
    fmt  
    spdlog 
    gflags
    OpenSSL::SSL 
    OpenSSL::Crypto 
    sqlite3
    Threads::Threads
)