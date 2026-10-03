# C++ LLM SDK & AI ChatServer
> **状态：** Completed · **版本：** v1.0.2 · **语言标准：** C++17
<p align="center">
  <img src="https://img.shields.io/badge/C++17-00599C?style=flat&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/CMake-064F8C?style=flat&logo=cmake&logoColor=white" />
  <img src="https://img.shields.io/badge/cpp--httplib-333333?style=flat&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/JsonCpp-8A2BE2?style=flat&logo=json&logoColor=white" />
  <img src="https://img.shields.io/badge/spdlog-4A5568?style=flat&logo=files&logoColor=white" />
  <img src="https://img.shields.io/badge/fmt-007ACC?style=flat&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/SQLite3-003B57?style=flat&logo=sqlite&logoColor=white" />
  <img src="https://img.shields.io/badge/Thread%20%2F%20Mutex%20%2F%20Atomic-00599C?style=flat&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/gflags-4285F4?style=flat&logo=google&logoColor=white" />
  <img src="https://img.shields.io/badge/OpenSSL-721412?style=flat&logo=openssl&logoColor=white" />
  <img src="https://img.shields.io/badge/GoogleTest-4285F4?style=flat&logo=googletest&logoColor=white" />
  <img src="https://img.shields.io/badge/HTTP%20%2B%20SSE-6B7280?style=flat&logo=fastapi&logoColor=white" />
  <img src="https://img.shields.io/badge/Ollama-000000?style=flat&logo=ollama&logoColor=white" />
</p>

&emsp;&emsp;基于 **C++17** 开发的多模型大语言模型接入SDK。从统一接口抽象出发，将不同大模型服务封装为独立 Provider，在上层提供统一的模型初始化、模型查询、普通对话和流式对话接口。并在 SDK 的基础上，项目进一步实现了一个基于 C++ 的 **AI Chat Server——[EchoMind]**，提供 Web 页面、会话管理、历史消息持久化、用户注册/登录以及 SSE 流式聊天等能力。


## 架构




## 仓库结构

```text
CPlusPlus_LLM_SDK/
├── ChatSDK/
│   ├── Include/
│   │   ├── ChatSDK.h
│   │   ├── LLMProvider.h
│   │   ├── LLMManager.h
│   │   ├── SessionManager.h
│   │   ├── UserManager.h
│   │   ├── DataManager.h
│   │   ├── Common.h
│   │   ├── CommonStruct.h
│   │   ├── DeepSeekProvider.h
│   │   ├── ChatGPTProvider.h
│   │   ├── KimiProvider.h
│   │   ├── OllamaProvider.h
│   │   └── Util/
│   │       └── LogModule.h
│   ├── Src/
│   │   ├── ChatSDK.cc
│   │   ├── LLMManager.cc
│   │   ├── SessionManager.cc
│   │   ├── UserManager.cc
│   │   ├── DataManager.cc
│   │   ├── DeepSeekProvider.cc
│   │   ├── ChatGPTProvider.cc
│   │   ├── KimiProvider.cc
│   │   ├── OllamaProvider.cc
│   │   └── Util/
│   │       └── LogModule.cc
│   └── Third_Party/
│       └── Httplib/
│           └── httplib.h
├── ChatServer/
│   ├── ChatServer.h
│   ├── ChatServer.cc
│   ├── Main.cc
│   ├── Daemon.hpp
│   ├── ChatServer.conf
│   ├── CMakeLists.txt
│   └── www/
├── SDKBuildTools/
│   ├── CMakeLists.txt
│   ├── install.sh
│   └── uninstall.sh
├── Test/
│   └── ...
├── Other/
│   └── TestTool/
├── DesignAndDocs/
└── README.md
```

## 快速开始

**环境要求：** Ubuntu 24.04+ · C++17 · CMake ≥ 3.10 · GCC / Clang
**主要依赖：** JsonCpp · spdlog · fmt · SQLite3 · OpenSSL · gflags · GoogleTest

### 1. SDK 安装

```bash
cd SDKBuildTools
./install.sh
```

编译并安装 `libChatSDK.a`、SDK 头文件及 `httplib.h`，默认安装至 `/usr/local/ChatSDK`。
卸载：`./uninstall.sh`

### 2. SDK 使用

通过环境变量配置云端模型 API Key：

```bash
export deepseek_apikey="YOUR_DEEPSEEK_API_KEY"
export chatgpt_apikey="YOUR_CHATGPT_API_KEY"
export KIMI_API_KEY="YOUR_KIMI_API_KEY"
```

```cpp
#include <ChatSDK.h>
using namespace Cplusplus_LLM_Provider;

ChatSDK sdk;
// 初始化模型配置后
sdk.initModels(configs);
auto sessionId = sdk.createSession("My Session", "deepseek-flash");
auto response = sdk.sendMessage(sessionId, "Hello!");
```

SDK 同时提供 `sendMessageStream()` 流式调用接口。

### 3. ChatServer 运行

先完成 SDK 安装，再编译服务器：

```bash
cd ChatServer //必须在该目录下运行
mkdir build ; cd build ; cmake .. ; cmake --build .
./build/AIChatServer --flagfile=ChatServer.conf
```



### 4. HTTP API
| Method | Endpoint | 功能 | Method | Endpoint | 功能 |
|---|---|---|---|---|---|
| POST | `/api/register` | 用户注册 / 登录 | POST | `/api/sessions` | 创建会话 |
| GET | `/api/sessionlists` | 获取会话列表 | GET | `/api/session/{id}/history` | 获取聊天历史 |
| GET | `/api/models` | 获取可用模型 | POST | `/api/message` | 发送消息 |
| POST | `/api/stream` | 流式对话（SSE） | POST | `/api/delsession` | 删除会话 |


## 核心技术设计

### 1. Provider 抽象与模型管理

通过 `LLMProvider` 抽象不同大模型平台的 API 差异，包括请求构造、HTTP 通信、响应解析及流式数据处理。

```text
                   ChatSDK
                      │
                  LLMManager
                      │
                 LLMProvider
          ┌───────────┼───────────┐
          ▼           ▼           ▼
      DeepSeek       Kimi       ChatGPT
                      ...
                   Ollama
```

`LLMManager` 使用 `std::unordered_map` 管理 Provider 实例及模型信息，统一处理模型初始化、可用性检查、普通请求和流式请求。
新增模型时，只需实现对应的 Provider 并接入管理层，减少模型 API 差异对上层业务的影响。

### 2. 流式响应

项目采用回调机制传递模型生成的增量内容，并通过 SSE 实现从模型 API 到前端的流式输出。

```text
LLM API → LLMProvider → LLMManager → ChatSDK → ChatServer → SSE → 前端页面
```

Provider 负责解析上游流式响应并提取文本，ChatServer 通过 `cpp-httplib` 的 `set_chunked_content_provider()` 持续向前端发送数据。
这种设计将模型通信与 HTTP 服务端的流式输出解耦。

### 3. Session 与 SQLite 数据管理

采用 `SessionManager` 与 `DataManager` 分离会话业务逻辑和数据库操作，使用 SQLite3 持久化用户、会话及消息数据。

```text
业务层 ChatSDK → 会话管理层 SessionManager → 数据管理层 DataManager → 数据库 SQLite3
```

### 4. SDK 静态库

ChatSDK 支持编译为 `libChatSDK.a`，并通过 `SDKBuildTools` 提供安装与卸载脚本。
安装后，上层 C++ 项目可以直接引用公开头文件：

```cpp
#include <ChatSDK.h>
```

SDK 将模型接入、会话管理等能力封装在统一接口后，调用方无需直接处理各模型平台的 API 细节。

## 测试

项目包含 Provider 单元测试与 ChatSDK 集成测试，覆盖模型初始化、可用性检查、会话管理、普通对话及流式输出。
部分测试需要配置云端模型 API Key；Ollama 测试需要本地服务正常运行。





