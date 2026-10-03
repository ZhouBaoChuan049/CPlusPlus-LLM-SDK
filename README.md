# CPlusPlus_LLM_SDK

> 一个基于 **C++17** 手写实现的大模型接入 SDK，并在 SDK 之上构建面向 Web 的网络智能聊天助手。

**C++17 · HTTP/HTTPS · JSON · SSE · 多模型接入 · 会话管理 · SQLite · Ollama**

---

## 1. 项目简介

`CPlusPlus_LLM_SDK` 是一个使用 C++ 编写的大模型接入与聊天服务项目。

项目没有直接依赖某一家厂商提供的 C++ SDK，而是从 **HTTP 请求、JSON 序列化、大模型接口协议、SSE 流式响应解析** 等基础能力出发，手写了一套统一的大模型接入层。

在此基础上，进一步构建了一个面向 Web 的智能聊天助手服务：

```text
                    ┌─────────────────────┐
                    │      Web Client     │
                    │   浏览器 / 前端页面   │
                    └──────────┬──────────┘
                               │ HTTP
                               ▼
                    ┌─────────────────────┐
                    │      ChatServer     │
                    │     Web 聊天服务      │
                    └──────────┬──────────┘
                               │
                               ▼
                    ┌─────────────────────┐
                    │      ChatSDK        │
                    │   统一聊天业务接口     │
                    └──────────┬──────────┘
                               │
               ┌───────────────┴────────────────┐
               │                                │
               ▼                                ▼
      ┌─────────────────┐              ┌─────────────────┐
      │    LLMManager   │              │ SessionManager  │
      │   大模型管理      │              │   会话管理       │
      └────────┬────────┘              └────────┬────────┘
               │                                │
        ┌──────┼──────┬──────┐                 ▼
        ▼      ▼      ▼      ▼          ┌──────────────┐
   DeepSeek  OpenAI  Kimi  Ollama        │   SQLite     │
                                      │ 会话/消息持久化 │
                                      └──────────────┘
```

整个项目可以分成两个层次：

**第一层：C++ LLM SDK**

负责解决“如何使用 C++ 接入不同的大模型”。

**第二层：ChatServer 智能聊天助手**

负责解决“如何真正把 SDK 变成一个可使用的网络聊天服务”。

---

## 2. 项目目标

这个项目主要围绕三个问题展开：

### 2.1 如何用 C++ 统一接入不同的大模型？

不同厂商的大模型 API 在：

- Endpoint
- 请求 JSON
- 鉴权方式
- 响应 JSON
- 流式响应格式

等方面存在差异。

项目通过抽象 `LLMProvider` 接口，将这些差异封装在具体 Provider 内部。

上层只需要面向统一接口编程。

---

### 2.2 如何处理大模型的流式响应？

大模型返回内容通常不是一次性返回，而是：

```text
data: {"choices":[{"delta":{"content":"你"}}]}

data: {"choices":[{"delta":{"content":"好"}}]}

data: {"choices":[{"delta":{"content":"！"}}]}

data: [DONE]
```

因此项目不仅实现普通同步请求，还实现了基于 **SSE（Server-Sent Events）** 的流式请求。

SDK 会完成：

```text
HTTP Response
      │
      ▼
接收网络数据
      │
      ▼
按 \n\n 划分 SSE 数据块
      │
      ▼
提取 data:
      │
      ▼
解析 JSON
      │
      ▼
提取 delta.content
      │
      ▼
通过 callback 返回给上层
```

这样 ChatServer 就可以继续将模型生成的内容以流式方式发送给 Web 前端，实现类似 ChatGPT 的逐字输出效果。

---

### 2.3 如何在 SDK 之上构建完整聊天系统？

SDK 并不是项目最终目的。

在 SDK 之上，继续实现：

```text
用户请求
   │
   ▼
ChatServer
   │
   ▼
ChatSDK
   │
   ├── 会话管理
   ├── 历史消息管理
   ├── 模型选择
   └── 大模型请求
          │
          ▼
      LLM Provider
          │
          ▼
      大模型服务
```

最终形成一个完整的网络智能聊天助手。

---

# 3. 核心特性

## 3.1 统一的 LLMProvider 抽象

项目定义统一的 Provider 接口：

```cpp
class LLMProvider
{
public:
    virtual ~LLMProvider() = default;

    virtual void InitModel(
        std::unordered_map<std::string, std::string> Config
    ) = 0;

    virtual bool IsModelAvailable() = 0;

    virtual std::string GetModelName() = 0;

    virtual ModelInfo GetModelDescription() = 0;

    virtual std::string SendMessages(
        std::vector<Message>& messages,
        std::unordered_map<std::string, std::string>& RequestPrograms
    ) = 0;

    virtual std::string SendMessagesAsStream(
        std::vector<Message>& messages,
        std::unordered_map<std::string, std::string>& RequestPrograms,
        func_t callback
    ) = 0;
};
```

因此，上层不需要关心具体的大模型厂商。

例如：

```text
ChatSDK
   │
   ▼
LLMManager
   │
   ├── DeepSeekProvider
   ├── ChatGPTProvider
   ├── KimiProvider
   └── OllamaProvider
```

新增模型时，只需要实现新的 Provider。

---

## 3.2 同步 + 流式两种调用方式

SDK 同时支持：

### 普通请求

```cpp
SendMessages(...)
```

模型生成完成后一次性返回结果。

### 流式请求

```cpp
SendMessagesAsStream(...)
```

模型生成内容通过 callback 持续返回：

```cpp
callback(content, false);
```

当收到：

```text
[DONE]
```

时：

```cpp
callback("[DONE]", true);
```

这种设计使 Provider 与上层业务之间保持松耦合。

---

## 3.3 多模型接入

项目采用 Provider 多态设计，可以接入不同的大模型服务。

当前项目围绕云端 API 与本地 Ollama 模型进行统一封装，包括：

| Provider | 类型 | 通信方式 |
|---|---|---|
| DeepSeek | 云端模型 | HTTP/HTTPS |
| ChatGPT / OpenAI | 云端模型 | HTTP/HTTPS |
| Kimi | 云端模型 | HTTP/HTTPS |
| Ollama | 本地模型 | HTTP |

不同模型的 API 差异被封装在 Provider 内部。

---

## 3.4 HTTP / JSON / SSE 协议级实现

项目底层使用 `cpp-httplib` 完成 HTTP/HTTPS 通信，并使用 JSON 完成请求与响应数据处理。

以流式请求为例：

```text
构造 Request
     │
     ▼
设置 HTTP Header
     │
     ├── Content-Type: application/json
     ├── Authorization: Bearer xxx
     └── Accept: text/event-stream
     │
     ▼
POST /v1/chat/completions
     │
     ▼
接收 Response Stream
     │
     ▼
解析 SSE
     │
     ▼
解析 JSON
     │
     ▼
提取增量内容
```

项目因此不仅涉及“大模型调用”，也涉及实际的网络编程与协议处理。

---

# 4. 项目整体架构

## 4.1 分层结构

```text
┌─────────────────────────────────────┐
│             Application             │
│                                     │
│             ChatServer              │
│        网络智能聊天助手服务           │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│               ChatSDK               │
│        对外提供统一聊天能力           │
└──────────────────┬──────────────────┘
                   │
        ┌──────────┴──────────┐
        ▼                     ▼
┌───────────────┐      ┌────────────────┐
│  LLMManager   │      │ SessionManager │
│ 大模型管理      │      │   会话管理       │
└───────┬───────┘      └───────┬────────┘
        │                      │
        ▼                      ▼
┌───────────────┐      ┌────────────────┐
│   Provider    │      │     SQLite     │
│   多模型适配    │      │  数据持久化      │
└───────┬───────┘      └────────────────┘
        │
   ┌────┼────┬────┐
   ▼    ▼    ▼    ▼
DeepSeek OpenAI Kimi Ollama
```

---

## 4.2 LLMManager

`LLMManager` 负责统一管理不同的大模型 Provider。

其核心职责包括：

- 管理已经注册的大模型
- 根据模型名称获取对应 Provider
- 检查模型是否可用
- 屏蔽不同 Provider 的调用差异
- 为 ChatSDK 提供统一的大模型访问入口

例如：

```text
ChatSDK
   │
   │ "kimi-k3"
   ▼
LLMManager
   │
   ▼
KimiProvider
   │
   ▼
Kimi API
```

而切换到其他模型时：

```text
ChatSDK
   │
   │ "deepseek"
   ▼
LLMManager
   │
   ▼
DeepSeekProvider
   │
   ▼
DeepSeek API
```

上层业务逻辑无需改变。

---

# 5. SessionManager 会话管理

一个真正的聊天系统不能只处理单条消息。

例如：

```text
用户：我叫小明
AI：你好，小明！

用户：我刚才告诉你我叫什么？
AI：你刚才告诉我，你叫小明。
```

第二次请求必须携带之前的上下文。

项目通过 Session 管理整个对话上下文：

```text
Session
 ├── SessionID
 ├── ModelName
 ├── Message 1
 ├── Message 2
 ├── Message 3
 └── ...
```

核心数据结构：

```cpp
class Session
{
public:
    std::string _SessionID;
    std::string _ModelNameUsed;
    std::vector<Message> _Messages;
    std::time_t _TimeCreate;
    std::time_t _LastTime;
};
```

因此每一个会话都拥有独立的历史消息列表。

---

# 6. SQLite 持久化

如果所有会话只保存在内存中，那么服务器重启以后历史记录就会丢失。

因此项目进一步引入 SQLite，对聊天数据进行持久化。

整体结构：

```text
ChatServer
    │
    ▼
ChatSDK
    │
    ▼
SessionManager
    │
    ▼
DataManager
    │
    ▼
SQLite
```

这样可以实现：

- 创建会话
- 查询历史会话
- 保存聊天消息
- 恢复历史上下文
- 服务重启后的数据保留

---

# 7. Web 智能聊天助手

在 SDK 完成以后，项目进一步实现了 ChatServer。

ChatServer 作为应用层服务，通过 HTTP 与 Web 前端进行通信。

整体请求链路：

```text
Browser
   │
   │ HTTP Request
   ▼
ChatServer
   │
   ▼
ChatSDK
   │
   ├── SessionManager
   │        │
   │        └── 获取历史消息
   │
   └── LLMManager
            │
            └── 选择 Provider
                    │
                    ▼
               LLM API
                    │
                    ▼
              SSE Stream
                    │
                    ▼
              ChatServer
                    │
                    ▼
                 Browser
```

流式场景下：

```text
LLM
 │
 ├── "你"
 ├── "好"
 ├── "，"
 ├── "我"
 └── "是 AI"
       │
       ▼
   callback()
       │
       ▼
 ChatServer
       │
       ▼
 HTTP Chunked Response
       │
       ▼
    Browser
```

因此用户无需等待整个模型回答生成完成，而是可以实时看到输出内容。

---

# 8. SSE 流式解析

项目针对 SSE 响应进行了专门处理。

一个典型响应：

```text
data: {"choices":[{"delta":{"content":"你好"}}]}

data: {"choices":[{"delta":{"content":"！"}}]}

data: [DONE]
```

处理流程：

```text
RecvBuffer
    │
    ▼
拼接到 buffer
    │
    ▼
查找 "\n\n"
    │
    ▼
提取完整 SSE 数据块
    │
    ▼
查找 "data:"
    │
    ▼
判断是否 [DONE]
    │
    ├── 是 → 结束流
    │
    └── 否
         │
         ▼
       JSON Parse
         │
         ▼
 choices[0]
         │
         ▼
       delta
         │
         ▼
     content
         │
         ▼
      callback
```

这个过程也解决了一个实际网络编程中的问题：

**一次网络接收不一定对应一个完整的 SSE 消息。**

因此不能简单地认为：

```cpp
recv() == 一条完整消息
```

而需要使用缓冲区对 TCP/HTTP 层收到的数据进行重新组包，再进行应用层协议解析。

---

# 9. 日志系统

项目使用 `spdlog` 构建日志系统，并在此基础上进行了封装。

提供：

```text
TRACE
DEBUG
INFO
WARNING
ERROR
CRITICAL
```

等日志级别。

示例：

```cpp
LogModule::INFO("Get Response Success!");
```

日志中同时保留：

```text
文件名
行号
日志级别
日志内容
```

方便定位网络请求、模型调用和服务端业务问题。

---

# 10. 目录结构

当前仓库主要目录：

```text
CPlusPlus_LLM_SDK/
│
├── Include/
│   └── 项目头文件
│
├── Src/
│   └── 项目源文件
│
├── Test/
│   └── 各模块测试代码
│
├── TestTool/
│   └── 测试与调试辅助程序
│
├── Third_Party/
│   └── Httplib/
│       └── cpp-httplib
│
└── README.md
```

整体遵循：

```text
Header
   ↓
Implementation
   ↓
Test
   ↓
Application
```

的组织方式。

---

# 11. 技术栈

| 技术 | 用途 |
|---|---|
| C++17 | 核心开发语言 |
| cpp-httplib | HTTP / HTTPS 网络通信 |
| JsonCpp | JSON 序列化与反序列化 |
| spdlog | 日志系统 |
| SQLite | 会话与消息持久化 |
| Ollama | 本地大模型运行环境 |
| HTTP / HTTPS | 模型 API 通信 |
| SSE | 大模型流式响应 |
| Linux / WSL2 | 主要开发与测试环境 |

---

# 12. 一个 Provider 是怎么实现的？

以 Kimi Provider 为例。

初始化：

```cpp
std::unordered_map<std::string, std::string> Config;

Config["_ApiKey"] = apiKey;
Config["_APIAccessAddress"] = "https://api.moonshot.cn";

provider.InitModel(Config);
```

普通请求：

```text
Message
   ↓
Serialize()
   ↓
JSON Request Body
   ↓
HTTP POST
   ↓
Deserialize()
   ↓
Response Content
```

流式请求：

```text
Message
   ↓
Serialize()
   ↓
stream = true
   ↓
HTTP POST
   ↓
SSE Stream
   ↓
content_receiver
   ↓
JSON Parse
   ↓
callback(chunk, false)
   ↓
[DONE]
   ↓
callback("[DONE]", true)
```

因此新增 Provider 的本质就是：

```text
统一接口
   +
模型专属协议适配
```

---

# 13. 如何扩展新的大模型？

只需要继承：

```cpp
class LLMProvider
```

然后实现：

```cpp
class MyProvider : public LLMProvider
{
public:
    void InitModel(
        std::unordered_map<std::string, std::string> Config
    ) override;

    bool IsModelAvailable() override;

    std::string GetModelName() override;

    ModelInfo GetModelDescription() override;

    std::string SendMessages(
        std::vector<Message>& messages,
        std::unordered_map<std::string, std::string>& RequestPrograms
    ) override;

    std::string SendMessagesAsStream(
        std::vector<Message>& messages,
        std::unordered_map<std::string, std::string>& RequestPrograms,
        func_t callback
    ) override;
};
```

Provider 内部只需要处理：

```text
1. 请求参数组织
2. JSON 序列化
3. HTTP 请求
4. HTTP Header
5. 普通响应解析
6. 流式响应解析
```

其余业务逻辑由 SDK 统一处理。

这也是整个项目最核心的扩展点。

---

# 14. 项目运行逻辑

一次完整的用户聊天请求大致经历：

```text
                User
                  │
                  ▼
          ┌───────────────┐
          │   Web Client  │
          └───────┬───────┘
                  │
             HTTP Request
                  │
                  ▼
          ┌───────────────┐
          │  ChatServer   │
          └───────┬───────┘
                  │
                  ▼
          ┌───────────────┐
          │    ChatSDK    │
          └───────┬───────┘
                  │
         ┌────────┴────────┐
         ▼                 ▼
 SessionManager       LLMManager
         │                 │
         ▼                 ▼
    History Data        Provider
                           │
                           ▼
                     LLM API / Ollama
                           │
                           ▼
                       SSE Stream
                           │
                           ▼
                      ChatServer
                           │
                           ▼
                    Browser Streaming
```

---

# 15. 项目重点

这个项目真正希望解决的，不只是：

> “如何调用一个大模型 API？”

而是：

> **如何用 C++ 把一个大模型 API，从底层网络请求封装成可复用的 SDK，再进一步落地为一个真正可以工作的网络聊天服务。**

因此项目从：

```text
HTTP 请求
   ↓
JSON
   ↓
SSE
   ↓
Provider
   ↓
LLMManager
   ↓
SessionManager
   ↓
ChatSDK
   ↓
ChatServer
```

逐步构建完整链路。

这也是项目与单纯“大模型 API Demo”最大的区别。

---

# 16. 开发环境

推荐使用 Linux / WSL2 环境进行开发与测试。

基础环境：

```text
C++17
GCC / G++
CMake / Make
OpenSSL
SQLite3
JsonCpp
spdlog
cpp-httplib
```

如果使用 Ollama，则还需要本地运行 Ollama 服务以及对应模型。

---

# 17. API Key 配置

项目中的 Provider 使用统一配置方式，例如：

```cpp
std::unordered_map<std::string, std::string> Config;

Config["_ApiKey"] = apiKey;
Config["_APIAccessAddress"] =
    "https://api.moonshot.cn";

provider.InitModel(Config);
```

实际部署时建议通过环境变量或独立配置文件提供 API Key：

```bash
export KIMI_API_KEY="your_api_key"
```

不要将真实 API Key 直接提交到 Git 仓库。

---

# 18. 测试

项目在 `Test/` 中保留了独立的模块测试代码，用于测试：

- Provider 初始化
- 模型信息获取
- 普通请求
- 流式请求
- SSE 数据解析
- 异常情况

建议先完成单个 Provider 的测试，再将其接入 `LLMManager` 和 `ChatSDK`。

---

# 19. 当前项目定位

```text
                  CPlusPlus_LLM_SDK
                           │
             ┌─────────────┴─────────────┐
             │                           │
             ▼                           ▼
      C++ LLM SDK                  ChatServer
             │                           │
             │                           │
       多模型接入                    Web 聊天助手
             │                           │
             ▼                           ▼
       Provider Layer              Application Layer
             │                           │
             └─────────────┬─────────────┘
                           ▼
                    完整 AI 应用链路
```

它既可以作为一个独立的 C++ LLM SDK 继续扩展，也可以作为上层 AI 应用的基础设施。

---

# 20. Roadmap

后续计划：

- [ ] 增加更多 LLM Provider
- [ ] 完善统一模型配置机制
- [ ] 完善异常处理与错误码体系
- [ ] 增加超时、重试机制
- [ ] 增加异步请求能力
- [ ] 完善 ChatServer 的用户体系
- [ ] 增强会话与消息管理
- [ ] 增加 Tool Calling / Function Calling
- [ ] 进一步完善 SDK 安装与部署方式
- [ ] 将 SDK 与 ChatServer 进行更明确的工程解耦

---

# 21. 项目结构思想

整个项目遵循一个比较明确的设计思路：

```text
底层：
网络通信、JSON、SSE
        ↓
基础能力：
Provider
        ↓
核心能力：
LLMManager + SessionManager
        ↓
SDK：
ChatSDK
        ↓
应用：
ChatServer
        ↓
最终产品：
Web 智能聊天助手
```

从底层协议到上层应用逐层抽象，尽量让每一层只负责自己的事情。

---

## 22. License

本项目用于 C++、网络编程以及大模型应用开发学习与工程实践。

具体开源许可协议以仓库后续发布的 `LICENSE` 文件为准。

---

## 23. Repository

GitHub：

[ZhouBaoChuan049/CPlusPlus_LLM_SDK](https://github.com/ZhouBaoChuan049/CPlusPlus_LLM_SDK)

---

> **C++ × LLM × Network Programming**
>
> 不只是调用大模型，而是用 C++ 把大模型真正接进一个完整的应用系统。