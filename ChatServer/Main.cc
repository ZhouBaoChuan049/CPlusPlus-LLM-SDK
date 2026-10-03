#include <gflags/gflags.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include "Daemon.hpp"
#include "ChatServer.h"

DEFINE_string(host, "0.0.0.0", "服务器绑定的地址");
DEFINE_int32(port, 8080, "服务器端口号");
DEFINE_string(log_level, "INFO", "日志级别: TRACE/DEBUG/INFO/WARNING/ERROR/CRITICAL");
DEFINE_double(temperature, 0.7, "模型温度值(0~2)");
DEFINE_int32(max_tokens, 2048, "最大token数");

DEFINE_string(deepseek_model, "deepseek-flash", "DeepSeek 模型名称");
DEFINE_string(chatgpt_model, "gpt-5.5", "ChatGPT 模型名称");
DEFINE_string(kimi_model, "kimi-k2.6", "Kimi 模型名称");

DEFINE_string(ollama_model, "deepseek-r1:1.5b", "Ollama 模型名称");
DEFINE_string(ollama_model_desc, "本地 Ollama 模型", "Ollama 模型描述");
DEFINE_string(ollama_endpoint, "http://127.0.0.1:11434", "Ollama 服务地址");
DEFINE_int32(ollama_num_ctx, 2048, "Ollama 上下文窗口大小");

static std::string GetEnv(const char* name)
{
    const char* value = std::getenv(name);
    return (value != nullptr && value[0] != '\0') ? std::string(value) : std::string();
}

static void PrintHelp(const char* program)
{
    std::cout
        << "AIChatServer - 基于 ChatSDK 的多模型 LLM 对话服务器\n\n"
        << "用法:\n"
        << "  " << program << " [选项]\n\n"
        << "选项:\n"
        << "  --host=<地址>         服务器绑定地址 (默认: " << FLAGS_host << ")\n"
        << "  --port=<端口>         服务器端口号 (默认: " << FLAGS_port << ")\n"
        << "  --log_level=<级别>    日志级别 TRACE/DEBUG/INFO/WARNING/ERROR/CRITICAL (默认: " << FLAGS_log_level << ")\n"
        << "  --temperature=<值>    模型温度值 0~2 (默认: " << FLAGS_temperature << ")\n"
        << "  --max_tokens=<值>     最大token数 (默认: " << FLAGS_max_tokens << ")\n"
        << "  --deepseek_model=<名> DeepSeek 模型名称 (默认: " << FLAGS_deepseek_model << ")\n"
        << "  --chatgpt_model=<名>  ChatGPT 模型名称 (默认: " << FLAGS_chatgpt_model << ")\n"
        << "  --kimi_model=<名>     Kimi 模型名称 (默认: " << FLAGS_kimi_model << ")\n"
        << "  --ollama_model=<名>   Ollama 模型名称 (默认: " << FLAGS_ollama_model << ")\n"
        << "  --ollama_model_desc=<描述> Ollama 模型描述 (默认: " << FLAGS_ollama_model_desc << ")\n"
        << "  --ollama_endpoint=<地址>  Ollama 服务地址 (默认: " << FLAGS_ollama_endpoint << ")\n"
        << "  --ollama_num_ctx=<值>  Ollama 上下文窗口 (默认: " << FLAGS_ollama_num_ctx << ")\n"
        << "  --flagfile=<文件>     从配置文件加载参数（命令行参数优先于配置文件）\n\n"
        << "环境变量(用于 API Key):\n"
        << "  deepseek_apikey   DeepSeek API Key\n"
        << "  chatgpt_apikey    ChatGPT API Key\n"
        << "  KIMI_API_KEY      Kimi API Key\n\n"
        << "使用案例:\n"
        << "  " << program << " --port=9090 --temperature=0.8\n"
        << "  " << program << " --flagfile=ChatServer.conf\n\n"
        << "提供的接口:\n"
        << "  GET  /pages/home.html          主页\n"
        << "  GET  /pages/register.html      注册页\n"
        << "  GET  /pages/chatroom.html      聊天室页面\n"
        << "  GET  /images/*                 静态图片资源\n"
        << "  GET  /api/sessions             创建会话\n"
        << "  GET  /api/sessionlists         获取会话列表\n"
        << "  GET  /api/session/{id}/history 获取会话历史消息\n"
        << "  GET  /api/models               获取可用模型列表\n"
        << "  POST /api/message              发送消息(全量返回)\n"
        << "  POST /api/stream               发送消息(流式返回)\n"
        << "  POST /api/delsession           删除会话\n";
}

static void PrintVersion()
{
    std::cout << "AIChatServer version 1.0.0\n";
}

static bool ReadApiKeys(ChatServerModule::SdkConfig& skdconfigs)
{
    skdconfigs._DeepseekApiKey = GetEnv("deepseek_apikey");
    skdconfigs._ChatGPTApiKey = GetEnv("chatgpt_apikey");
    skdconfigs._KimiApiKey = GetEnv("KIMI_API_KEY");
    return true;
}

static bool ValidateConfig(const ChatServerModule::SdkConfig& skdconfigs)
{
    if (skdconfigs._Temperature < 0.0 || skdconfigs._Temperature > 2.0){
        std::cerr << "参数错误: temperature 必须在 0~2 之间, 当前值: "
                  << skdconfigs._Temperature << std::endl;
        return false;
    }
    if (skdconfigs._MaxTokens < 0){
        std::cerr << "参数错误: max_tokens 不能为负数, 当前值: "
                  << skdconfigs._MaxTokens << std::endl;
        return false;
    }
    if (skdconfigs._DeepseekApiKey.empty() &&
         skdconfigs._ChatGPTApiKey.empty() &&
          skdconfigs._KimiApiKey.empty()){
        std::cerr << "参数错误: 至少需要提供一个非空的云端模型 API Key"
                  << " (deepseek_apikey / chatgpt_apikey / KIMI_API_KEY)" << std::endl;
        return false;
    }
    if (skdconfigs._OllamaModelName.empty() ||
         skdconfigs._OllamaModelDesc.empty() ||
          skdconfigs._OllamaEndpoint.empty() ||
           skdconfigs._OllamaNumCtx <= 0){
        std::cerr << "参数错误: Ollama 配置参数都不能为空" << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    DaemonModule::EnableDaemon(1,0);//守护化(不 chdir，需从 ChatServer/ 目录启动)
    for (int i = 1; i < argc; ++i){
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help"){
            PrintHelp(argv[0]);
            return 0;
        }
        if (arg == "-v" || arg == "--version"){
            PrintVersion();
            return 0;
        }
    }

    bool hasFlagfile = false;
    for (int i = 1; i < argc; ++i){
        std::string arg = argv[i];
        if (arg.rfind("--flagfile=", 0) == 0 || arg.rfind("-flagfile=", 0) == 0){
            hasFlagfile = true;
            break;
        }
    }
    if (!hasFlagfile){
        std::ifstream conf("ChatServer.conf");
        if (conf.good()){
            gflags::ReadFromFlagsFile("ChatServer.conf", argv[0], true);
        }
    }

    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    ChatServerModule::SdkConfig skdconfigs;
    skdconfigs._Temperature = FLAGS_temperature;
    skdconfigs._MaxTokens = FLAGS_max_tokens;
    skdconfigs._DeepseekModelName = FLAGS_deepseek_model;
    skdconfigs._ChatGPTModelName = FLAGS_chatgpt_model;
    skdconfigs._KimiModelName = FLAGS_kimi_model;
    skdconfigs._OllamaModelName = FLAGS_ollama_model;
    skdconfigs._OllamaModelDesc = FLAGS_ollama_model_desc;
    skdconfigs._OllamaEndpoint = FLAGS_ollama_endpoint;
    skdconfigs._OllamaNumCtx = FLAGS_ollama_num_ctx;
    ReadApiKeys(skdconfigs);

    if (!ValidateConfig(skdconfigs)){
        return 1;
    }

    ChatServerModule::ChatServerConfig serverConfigs;
    serverConfigs._host = FLAGS_host;
    serverConfigs._port = FLAGS_port;

    ChatServerModule::ChatServer server;
    if (!server.InitChatServer(skdconfigs)){
        std::cerr << "ChatServer 初始化失败!" << std::endl;
        return 1;
    }
    if (!server.Start(serverConfigs)){
        std::cerr << "ChatServer 启动失败!" << std::endl;
        return 1;
    }
    std::cout << "AIChatServer 已启动: " << FLAGS_host << ":" << FLAGS_port << std::endl;

    pause();
    server.Stop();
    return 0;
}