#include <gflags/gflags.h>

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

#include "ChatServer.h"

// ==================== gflags 参数定义 ====================
// 服务器监听地址与端口
DEFINE_string(host, "0.0.0.0", "服务器绑定的地址");
DEFINE_int32(port, 8080, "服务器绑定的端口号");

// 日志级别
DEFINE_string(log_level, "INFO",
              "日志级别，可选: TRACE/DEBUG/INFO/WARNING/ERROR/CRITICAL/OFF");

// 模型采样参数
DEFINE_double(temperature, 0.7, "模型采样温度，取值范围 [0.0, 2.0]");
DEFINE_int32(max_tokens, 2048, "模型单次生成的最大 token 数（不能为负数）");

// Ollama 本地模型配置（安全检查要求不能为空）
DEFINE_string(ollama_model_name, "", "Ollama 本地模型名称");
DEFINE_string(ollama_model_desc, "", "Ollama 本地模型描述");
DEFINE_string(ollama_endpoint, "", "Ollama 服务地址，例如 127.0.0.1:11434");

// ==================== 版本号 ====================
static const char* kVersion = "1.0.0";

// ==================== 帮助信息 ====================
static const char* kUsage = R"(
AIChatServer —— 基于 ChatSDK 的多模型 HTTP 聊天服务

一、参数选项说明:
  --host=<addr>               服务器绑定地址 (默认 0.0.0.0)
  --port=<port>               服务器端口号 (默认 8080)
  --log_level=<level>         日志级别 TRACE/DEBUG/INFO/WARNING/ERROR/CRITICAL/OFF (默认 INFO)
  --temperature=<value>       采样温度，范围 [0.0, 2.0] (默认 0.7)
  --max_tokens=<n>            最大 token 数，非负数 (默认 2048)
  --ollama_model_name=<name>  Ollama 本地模型名称 (不能为空)
  --ollama_model_desc=<desc>  Ollama 本地模型描述 (不能为空)
  --ollama_endpoint=<addr>    Ollama 服务地址 (不能为空)
  --flagfile=<file>           从配置文件加载以上参数
  -h, --help                  显示本帮助信息
  -v, --version               显示版本号

  说明:
  - deepseek / chatgpt / kimi 的 apikey 不从命令行读取，而是直接从环境变量获取:
      deepseek_apikey  -> deepseek 模型
      chatgpt_apikey   -> chatgpt 模型
      KIMI_API_KEY     -> kimi 模型
    至少需要提供一个非空的 apikey。

二、使用案例:
  ./AIChatServer
  ./AIChatServer --host=0.0.0.0 --port=8080 --log_level=INFO
  ./AIChatServer --flagfile=ChatServer.conf
  ./AIChatServer --version

三、ChatServer 提供的接口:
  POST   /api/session                创建会话
  GET    /api/sessions               获取会话列表
  GET    /api/models                 获取可用模型列表
  DELETE /api/session/{id}           删除指定会话
  GET    /api/session/{id}/history   获取指定会话的历史消息
  POST   /api/message                发送消息（全量返回）
  POST   /api/message/async          发送消息（流式/增量返回）
)";

namespace
{
    spdlog::level::level_enum ParseLogLevel(const std::string& level)
    {
        std::string upper;
        upper.reserve(level.size());
        for (char c : level)
            upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));

        if (upper == "TRACE") return spdlog::level::trace;
        if (upper == "DEBUG") return spdlog::level::debug;
        if (upper == "INFO") return spdlog::level::info;
        if (upper == "WARN" || upper == "WARNING") return spdlog::level::warn;
        if (upper == "ERR" || upper == "ERROR") return spdlog::level::err;
        if (upper == "CRITICAL") return spdlog::level::critical;
        if (upper == "OFF") return spdlog::level::off;
        return spdlog::level::info;  // 默认 INFO
    }

    std::string GetEnv(const char* name)
    {
        const char* value = std::getenv(name);
        return value == nullptr ? std::string() : std::string(value);
    }
}

int main(int argc, char** argv)
{
    // 让 gflags 的 --help 显示我们自定义的使用说明
    gflags::SetUsageMessage(kUsage);

    // 先处理 -h/--help、-v/--version 短选项（gflags 原生不提供 -h / -v）
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            std::cout << kUsage << std::endl;
            return 0;
        }
        if (arg == "-v" || arg == "--version")
        {
            std::cout << "AIChatServer version " << kVersion << std::endl;
            return 0;
        }
    }

    // 解析命令行参数（同时支持 --flagfile=ChatServer.conf 加载配置文件）
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // ==================== 从环境变量读取 apikey ====================
    const std::string deepseekKey = GetEnv("deepseek_apikey");
    const std::string chatgptKey = GetEnv("chatgpt_apikey");
    const std::string kimiKey = GetEnv("KIMI_API_KEY");

    // ==================== 安全检查 ====================
    // 1. 温度值必须在 [0, 2] 之间
    if (FLAGS_temperature < 0.0 || FLAGS_temperature > 2.0)
    {
        std::cerr << "[Error] temperature 必须在 [0.0, 2.0] 之间，当前值: "
                  << FLAGS_temperature << std::endl;
        return 1;
    }
    // 2. 最大 token 数不能为负数
    if (FLAGS_max_tokens < 0)
    {
        std::cerr << "[Error] max_tokens 不能为负数，当前值: "
                  << FLAGS_max_tokens << std::endl;
        return 1;
    }
    // 3. 至少有一个 apikey 不为空
    if (deepseekKey.empty() && chatgptKey.empty() && kimiKey.empty())
    {
        std::cerr << "[Error] 至少需要提供一个非空的云端模型 apikey"
                     " (deepseek_apikey / chatgpt_apikey / KIMI_API_KEY)" << std::endl;
        return 1;
    }
    // 4. ollama 配置参数都不能为空
    if (FLAGS_ollama_model_name.empty() || FLAGS_ollama_model_desc.empty() ||
        FLAGS_ollama_endpoint.empty())
    {
        std::cerr << "[Error] ollama 配置参数不能为空"
                     " (ollama_model_name / ollama_model_desc / ollama_endpoint)" << std::endl;
        return 1;
    }

    // ==================== 初始化日志 ====================
    LogModule::SpdLogPack::SpdLogInit(
        "AIChatServer",
        "AIChatServer.log",
        ParseLogLevel(FLAGS_log_level),
        CONSOLE__MODE);

    // ==================== 构造配置并启动服务 ====================
    ChatServerModule::ServerConfig config;
    config.host = FLAGS_host;
    config.port = FLAGS_port;
    config.logLevel = FLAGS_log_level;
    config.temperature = FLAGS_temperature;
    config.maxTokens = FLAGS_max_tokens;

    config.deepseekAPIKey = deepseekKey;
    config.chatGPTAPIKey = chatgptKey;
    // 注意: ChatServer.cc 中 kimi 模型的 apikey 读取的是 geminiAPIKey 字段
    config.geminiAPIKey = kimiKey;

    config.ollamaModelName = FLAGS_ollama_model_name;
    config.ollamaModelDesc = FLAGS_ollama_model_desc;
    config.ollamaEndpoint = FLAGS_ollama_endpoint;

    ChatServerModule::ChatServer server(config);
    if (!server.Start())
    {
        std::cerr << "[Error] AIChatServer 启动失败" << std::endl;
        return 1;
    }

    std::cout << "\nAIChatServer 已启动，监听 " << config.host << ":" << config.port
              << std::endl;
    std::cout << "输入 quit 或 exit 并回车以停止服务..." << std::endl;

    std::string cmd;
    while (std::getline(std::cin, cmd))
    {
        if (cmd == "quit" || cmd == "exit")
            break;
    }

    server.Stop();
    LogModule::INFO("AIChatServer 已停止");
    return 0;
}
