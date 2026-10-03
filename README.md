# C++ LLM SDK
## **A C++17 multi-model LLM SDK integrating cloud APIs and local Ollama models, with a unified interface and ChatPlatform built on top.**

> **状态：** Completed · **版本：** v1.0.0 · **语言标准：** C++17

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

&emsp;&emsp;**CPlusPlus_LLM_SDK** 是一个基于 **C++17** 开发的多模型大语言模型接入 SDK。
&emsp;&emsp;项目从统一接口抽象出发，将不同大模型服务封装为独立 Provider，在上层提供统一的模型初始化、模型查询、普通对话和流式对话接口。
&emsp;&emsp;在 SDK 的基础上，项目进一步实现了一个基于 C++ 的 **AI Chat Server**，提供 Web 页面、会话管理、历史消息持久化、用户注册/登录以及 SSE 流式聊天等能力。

# 项目目录结构

```text
CPlusPlus_LLM_SDK/
│
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
│   │
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
│   │
│   └── Third_Party/
│       └── Httplib/
│           └── httplib.h
│
├── ChatServer/
│   ├── ChatServer.h
│   ├── ChatServer.cc
│   ├── Main.cc
│   ├── Daemon.hpp
│   ├── ChatServer.conf
│   ├── CMakeLists.txt
│   └── www/
│
├── SDKBuildTools/
│   ├── CMakeLists.txt
│   ├── install.sh
│   └── uninstall.sh
│
├── Test/
│   └── ...
│
├── Other/
│   └── TestTool/
│
├── DesignAndDocs/
│
└── README.md
```
