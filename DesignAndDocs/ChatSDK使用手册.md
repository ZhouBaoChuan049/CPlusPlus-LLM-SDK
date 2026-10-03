# ChatSDK 使用手册

## 一、ChatSDK 介绍

ChatSDK 是一个基于 C++17 的多模型大语言模型（LLM）软件开发工具包。它将多种云端与本地大模型的调用统一封装为一致的接口，让开发者无需关心每种模型各自差异化的 HTTP 协议，即可快速接入对话能力。

ChatSDK 主要具备以下能力：

- **多模型统一接入**：内置对 DeepSeek、ChatGPT、Kimi 等云端模型的封装，并支持接入本地 Ollama 模型。
- **会话管理**：提供会话的创建、查询、列表、删除，以及历史消息的持久化（基于 SQLite）。
- **用户管理**：支持用户注册/查询，以及用户与会话的归属关系。
- **流式与非流式输出**：既支持一次性返回完整回答，也支持通过回调函数逐段接收流式输出（SSE）。
- **日志系统**：内置基于 spdlog 的日志模块，支持普通文件、控制台、循环日志、异步日志等多种模式。

代码位于命名空间 `Cplusplus_LLM_Provider` 中，核心入口类是 `ChatSDK`。

---

## 二、ChatSDK 的获取

通过 git 克隆仓库：

```bash
git clone https://github.com/ZhouBaoChuan049/CPlusPlus_LLM_SDK.git
```

克隆完成后进入项目目录：

```bash
cd CPlusPlus_LLM_SDK
```

---

## 三、ChatSDK 的目录结构

```
CPlusPlus_LLM_SDK/
├── ChatSDK/                          # SDK 核心源码
│   ├── Include/                      # 头文件
│   │   ├── ChatSDK.h                 # SDK 主类（对外入口）
│   │   ├── Common.h                  # 通用定义、日志模式宏、异常枚举
│   │   ├── CommonStruct.h            # 核心数据结构（Config/Message/Session/User 等）
│   │   ├── LLMProvider.h             # LLM 提供者抽象基类
│   │   ├── LLMManager.h              # 模型管理器（注册/查找/调度模型）
│   │   ├── SessionManager.h          # 会话管理器
│   │   ├── UserManager.h             # 用户管理器
│   │   ├── DataManager.h             # 数据持久化（SQLite）
│   │   ├── ChatGPTProvider.h         # ChatGPT 模型实现
│   │   ├── DeepSeekProvider.h        # DeepSeek 模型实现
│   │   ├── KimiProvider.h            # Kimi 模型实现
│   │   ├── OllamaProvider.h          # Ollama 本地模型实现
│   │   └── Util/LogModule.h          # 日志模块
│   ├── Src/                          # 实现源码（.cc）
│   └── Third_Party/Httplib/          # 第三方依赖 httplib.h（header-only）
│
├── ChatServer/                       # 基于 ChatSDK 的示例对话服务器
│   ├── Main.cc                       # 入口（守护化 + gflags 参数解析）
│   ├── ChatServer.cc / ChatServer.h  # HTTP 服务器实现
│   ├── Daemon.hpp                    # 守护进程模块
│   ├── ChatServer.conf               # 配置文件
│   ├── CMakeLists.txt                # 服务器构建脚本
│   └── www/                          # 前端页面（html/css/js/images）
│
├── SDKBuildTools/                    # SDK 构建与安装工具
│   ├── install.sh                    # 一键编译并安装 SDK
│   ├── uninstall.sh                  # 卸载 SDK
│   └── CMakeLists.txt                # SDK 构建脚本（生成静态库）
│
├── Test/                             # 测试用例
│   ├── ChatGPTTest/                  # ChatGPT 模型测试
│   └── KimiTest/                     # Kimi 模型测试
│
└── DesignAndDocs/                    # 设计与说明文档
```

---

## 四、部署上云方案

以下是从用户视角、将项目完整部署到云服务器并运行的流程。

### 第 1 步：获取代码并克隆到云服务器

```bash
git clone https://github.com/ZhouBaoChuan049/CPlusPlus_LLM_SDK.git
cd CPlusPlus_LLM_SDK
```

### 第 2 步：安装并配置编译环境

安装编译工具与依赖库（以 Debian/Ubuntu 为例）：

```bash
sudo apt update
sudo apt install -y build-essential cmake g++ git \
    libjsoncpp-dev libfmt-dev libspdlog-dev libsqlite3-dev \
    libssl-dev libgflags-dev
```

### 第 3 步：手动配置环境变量（API Key）

SDK 通过环境变量读取云端模型的 API Key。启动服务前需设置（至少提供一个非空的云端 Key）：

```bash
export deepseek_apikey="你的 DeepSeek API Key"
export chatgpt_apikey="你的 ChatGPT API Key"
export KIMI_API_KEY="你的 Kimi API Key"
```

> 建议将上述行写入 `~/.bashrc` 或服务启动脚本，避免每次登录重复配置。

### 第 4 步：安装 SDK

执行一键安装脚本，编译静态库并安装到 `/usr/local/ChatSDK/`：

```bash
sudo bash SDKBuildTools/install.sh
```

安装成功后，头文件位于 `/usr/local/ChatSDK/include/`，静态库为 `/usr/local/ChatSDK/lib/libChatSDK.a`，第三方头文件位于 `/usr/local/ChatSDK/Third_Party/Httplib/`。

### 第 5 步：编译 ChatServer

SDK 安装完成后，构建示例服务器：

```bash
cd ChatServer
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

### 第 6 步：启动服务（守护化后台进程）

ChatServer 启动时会自动进入守护进程（后台运行），需在 `ChatServer/` 目录下执行：

```bash
cd /path/to/CPlusPlus_LLM_SDK/ChatServer
./build/AIChatServer --flagfile=ChatServer.conf
```

启动后进程转为后台守护进程，服务监听 `0.0.0.0:8080`，通过浏览器访问 `http://<服务器IP>:8080/pages/home.html` 即可。

> 参数说明：`ChatServer.conf` 中可配置监听地址、端口、日志级别、温度、最大 token 数，以及各模型名称与 Ollama 本地服务地址等。

### 第 7 步：停止守护进程

通过进程名查找并结束服务：

```bash
pkill -f AIChatServer
```

或先定位 PID 再结束：

```bash
pgrep -f AIChatServer   # 查看 PID
kill <PID>
```

### 第 8 步：卸载 SDK

```bash
sudo bash SDKBuildTools/uninstall.sh
```

该脚本会删除 `/usr/local/ChatSDK/` 安装目录。

---

## 五、接口介绍

以下接口均位于命名空间 `Cplusplus_LLM_Provider`，由类 `ChatSDK` 提供。

### 5.1 构造函数

```cpp
explicit ChatSDK(const std::string& dbname = "chat_sdk.db")
```

- 功能：创建 ChatSDK 实例，`dbname` 为会话/用户数据持久化所用的 SQLite 数据库文件名。
- 参数：`dbname` - 数据库文件名，缺省为 `chat_sdk.db`。
- 说明：构造函数内部初始化会话管理器 `SessionManager` 与用户管理器 `UserManager`。

### 5.2 初始化模型

```cpp
bool initModels(const std::vector<std::shared_ptr<Config>>& configs)
```

- 功能：初始化所支持的模型。
- 参数：`configs` - 所有支持的模型所需配置的参数（`APIConfig` 或 `OllamaConfig` 的 `shared_ptr` 列表）。
- 返回值：初始化成功返回 `true`，否则返回 `false`。

### 5.3 创建会话

```cpp
std::string createSession(const std::string SessionName, const std::string& modelName, const std::string userName = "")
```

- 功能：创建一个新的会话。
- 参数：
  - `SessionName` - 会话名称；
  - `modelName` - 该会话所使用的模型名称；
  - `userName` - 会话归属的用户名（可选，缺省为空串表示未归属）。
- 返回值：新会话的唯一 ID（`std::string`）；若模型未初始化则返回空串。

### 5.4 获取会话

```cpp
std::shared_ptr<Session> getSession(const std::string& sessionId)
```

- 功能：根据会话 ID 获取会话对象。
- 参数：`sessionId` - 会话 ID。
- 返回值：指向 `Session` 的智能指针；若会话不存在或模型未初始化返回 `nullptr`。

### 5.5 获取会话列表

```cpp
std::vector<std::string> getSessionLists() const
```

- 功能：获取当前所有会话的 ID 列表。
- 返回值：会话 ID 的 `vector`；若模型未初始化返回空列表。

### 5.6 删除会话

```cpp
bool deleteSession(const std::string& sessionId)
```

- 功能：删除指定会话。
- 参数：`sessionId` - 会话 ID。
- 返回值：删除成功返回 `true`，否则返回 `false`。

### 5.7 获取可用模型

```cpp
std::vector<std::pair<std::string, ModelInfo>> getAvailableModels()
```

- 功能：获取当前已注册/可用的模型列表。
- 返回值：`vector<pair>`，其中 key 为模型名称，value 为 `ModelInfo`（描述该模型的名称、描述、访问地址、可用状态）。

### 5.8 发送消息（非流式）

```cpp
std::string sendMessage(const std::string& sessionId, const std::string& message)
```

- 功能：向指定会话发送一条消息，并返回模型的完整回答。
- 参数：
  - `sessionId` - 会话 ID；
  - `message` - 用户输入的消息内容。
- 返回值：模型回答的完整文本；若会话/模型配置不存在则返回空串。

### 5.9 发送消息（流式）

```cpp
std::string sendMessageStream(const std::string& sessionId, const std::string& message,
                              std::function<void(const std::string&, bool)> callback)
```

- 功能：以流式方式向指定会话发送消息，逐段回调输出。
- 参数：
  - `sessionId` - 会话 ID；
  - `message` - 用户输入的消息内容；
  - `callback` - 回调函数，签名 `void(const std::string& fragment, bool is_final)`，`fragment` 为本段增量文本，`is_final` 表示是否为最后一段。
- 返回值：完整回答文本；若会话/模型配置不存在则返回空串。

---

## 六、数据结构说明

### Config / APIConfig / OllamaConfig

```cpp
struct Config {
    std::string _modelName;      // 模型名称
    double _temperature = 0.7;   // 温度（0~2）
    int _maxTokens = 2048;       // 最大 token 数
};

struct APIConfig : public Config {
    std::string _apiKey;         // 云端模型 API Key
};

struct OllamaConfig : public Config {
    std::string _modelDesc;      // 模型描述
    std::string _endpoint;       // Ollama 服务地址（需主动提供）
    int Num_Ctx;                 // 上下文窗口大小
};
```

### Message

```cpp
class Message {
    std::string _MessageID;
    std::string _SessionID;
    std::string _Role;     // "user" 或 "assistant"
    std::string _Content;  // 消息内容
    std::time_t _Time;
};
```

### ModelInfo

```cpp
class ModelInfo {
    std::string _ModelName;
    std::string _ModelDesc;
    std::string _APIAccessAddress;
    bool _IsThisModelAvailable;
};
```

### Session

```cpp
class Session {
    std::string _SessionID;
    std::string _ModelNameUsed;   // 该会话使用的模型
    std::string _UserName;        // 归属用户，空串表示未归属
    std::vector<Message> _Messages;
    std::time_t _TimeCreate;
    std::time_t _LastTime;
};
```

### User

```cpp
class User {
    std::string _UserName;
    std::string _PasswordHash;   // 密码哈希
    std::string _Salt;           // 密码哈希所用的盐
    std::vector<Session> _Sessions;
};
```

---

## 七、快速上手：代码示例

以下示例演示了「初始化模型 → 创建会话 → 流式发送消息」的完整流程。

```cpp
#include <ChatSDK.h>   // 安装后头文件位于 /usr/local/ChatSDK/include

#include <iostream>
#include <memory>
#include <vector>

using namespace Cplusplus_LLM_Provider;

int main()
{
    // 1. 创建 SDK 实例（指定数据库文件名）
    ChatSDK sdk("chat_sdk.db");

    // 2. 准备模型配置
    std::vector<std::shared_ptr<Config>> configs;

    auto deepseek = std::make_shared<APIConfig>();
    deepseek->_modelName = "deepseek-flash";
    deepseek->_apiKey = "你的 DeepSeek API Key";
    deepseek->_temperature = 0.7;
    deepseek->_maxTokens = 2048;
    configs.push_back(deepseek);

    // 如需接入本地 Ollama，可追加如下配置
    // auto ollama = std::make_shared<OllamaConfig>();
    // ollama->_modelName = "deepseek-r1:1.5b";
    // ollama->_modelDesc = "本地 Ollama 模型";
    // ollama->_endpoint = "http://127.0.0.1:11434";
    // ollama->Num_Ctx = 2048;
    // configs.push_back(ollama);

    // 3. 初始化模型
    if (!sdk.initModels(configs)) {
        std::cerr << "模型初始化失败" << std::endl;
        return 1;
    }

    // 4. 创建会话
    std::string sessionId = sdk.createSession("我的第一个会话", "deepseek-flash");
    if (sessionId.empty()) {
        std::cerr << "创建会话失败" << std::endl;
        return 1;
    }

    // 5. 流式发送消息
    std::cout << "模型回答：" << std::endl;
    sdk.sendMessageStream(
        sessionId,
        "你好，请介绍一下你自己。",
        [](const std::string& fragment, bool is_final) {
            std::cout << fragment;   // 逐段输出
            std::cout.flush();
            if (is_final) {
                std::cout << std::endl;
            }
        }
    );

    return 0;
}
```

### 编译与链接

将上述代码保存为 `main.cpp`，使用如下命令编译（需已通过 `install.sh` 安装 SDK）：

```bash
g++ main.cpp -std=c++17 -DCPPHTTPLIB_OPENSSL_SUPPORT \
    -I/usr/local/ChatSDK/include -L/usr/local/ChatSDK/lib -lChatSDK \
    -ljsoncpp -lfmt -lspdlog -lsqlite3 -lssl -lcrypto -lpthread -o MyApp
```

运行：

```bash
./MyApp
```

> 使用 CMake 构建时，需在 `target_link_libraries` 中链接 `ChatSDK jsoncpp fmt spdlog sqlite3 ssl crypto pthread`，并确保 `link_directories` 置于 `add_executable` 之前，且 `include_directories` 指向 `/usr/local/ChatSDK/include`。

---

## 附：注意事项

- 编译静态库与业务代码时，必须定义宏 `CPPHTTPLIB_OPENSSL_SUPPORT` 以启用 HTTPS 访问。
- 温度值必须在 0~2 之间，最大 token 数不能为负数。
- 至少提供一个非空的云端模型 API Key；Ollama 的各项配置参数均不能为空。
- 使用 `install.sh` 安装后，头文件统一通过 `#include <ChatSDK.h>` 引入。
- 服务端以守护进程方式运行时，需在 `ChatServer/` 目录下启动，以保证相对路径（数据库、日志、静态资源）正确。