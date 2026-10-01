#include <ChatSDK.h>
#include <cstdlib>
#include "../Third_Party/Httplib/httplib.h"
namespace ChatServerModule
{
    struct ServerConfig{
        std::string host = "0.0.0.0";    
        int port = 8080;                 
        std::string logLevel = "INFO";   

        double temperature = 0.7;        
        int maxTokens = 1024;            

        std::string deepseekAPIKey;    
        std::string geminiAPIKey;       
        std::string chatGPTAPIKey;      

        std::string ollamaModelName;    
        std::string ollamaModelDesc;   
        std::string ollamaEndpoint;     
    };

    class ChatServer
    {
    public:
        ChatServer(const ServerConfig configs);
        ~ChatServer();
        bool Start();
        void Stop();
        bool  IsRunning();
    private:
        std::string buildResponse(const std::string& message, bool success = false);
        // 处理创建会话请求
        void handleCreateSessionRequest(const httplib::Request& request, httplib::Response& response);
        // 处理获取会话列表请求
        void handleGetSessionListsRequest(const httplib::Request& request, httplib::Response& response);
        // 处理获取模型列表请求
        void handleGetModelListsRequest(const httplib::Request& request, httplib::Response& response);
        // 处理删除会话请求
        void handleDeleteSessionRequest(const httplib::Request& request, httplib::Response& response);
        // 处理获取历史消息请求
        void handleGetHistoryMessagesRequest(const httplib::Request& request, httplib::Response& response);
        // 处理发送消息请求-全量返回
        void handleSendMessageRequest(const httplib::Request& request, httplib::Response& response);
        // 处理发送消息请求-增量返回
        void handleSendMessageStreamRequest(const httplib::Request& request, httplib::Response& response);

        // 设置HTTP路由规则
        void setHttpRoutes();

    private:
        ServerConfig _configs;   // 服务器配置信息
        std::unique_ptr<httplib::Server> _chatServer = nullptr;   // HTTP服务器
        std::shared_ptr<Cplusplus_LLM_Provider::ChatSDK> _chatSDK = nullptr;   // 聊天SDK
        std::atomic<bool> _isRunning = {false};   // 是否正在运行
    };
}