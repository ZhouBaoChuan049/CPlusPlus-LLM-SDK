#include <ChatSDK.h>
#include "../ChatSDK/Third_Party/Httplib/httplib.h"
//三方依赖，我本来想把它放在SDK目录外面的，想来想去放在里面更合理一点。
using namespace Cplusplus_LLM_Provider ;
namespace ChatServerModule
{
    struct SdkConfig
    {
        //模型参数——共有
        double _Temperature ;    
        int _MaxTokens ;   
        //API接入的云端模型参数
        std::string _DeepseekApiKey;
        std::string _DeepseekModelName;
        std::string _ChatGPTApiKey;
        std::string _ChatGPTModelName;
        std::string _KimiApiKey;
        std::string _KimiModelName;
        //Ollama参数
        std::string _OllamaModelName;
        std::string _OllamaModelDesc;    
        std::string _OllamaEndpoint; 
        int _OllamaNumCtx;
    };
    struct ChatServerConfig
    {
        std::string _host = "0.0.0.0" ;
        int _port = 8080 ;
    };


    class ChatServer
    {
    public:
        ChatServer()
            :_isrunning(false),
             _chatsdk(nullptr),
             _chatserver(nullptr)
        {}
        bool InitChatServer(SdkConfig& configs);
        bool Start(ChatServerConfig& serverConfigs);
        bool Stop();
        ~ChatServer() = default ;
    private:
        void SendJsonResponse(httplib::Response& response,int status,bool success,
            const std::string& message,const Json::Value& data);
        //注册所有的路由方案
        void RouterRegister(); 
        //资源获取
        void HandleGetHomePage(const httplib::Request& request, httplib::Response& response);
        void HandleGetRegisterPage(const httplib::Request& request, httplib::Response& response);
        void HandleGetAuthorPage(const httplib::Request& request, httplib::Response& response);
        void HandleGetChatRoomPage(const httplib::Request& request, httplib::Response& response);
        void HandleImage(const httplib::Request& request, httplib::Response& response);
        //请求处理
        void HandleCreateSession(const httplib::Request& request, httplib::Response& response);
        void HandleGetSessionLists(const httplib::Request& request, httplib::Response& response);
        void HandleGetAvailableModel(const httplib::Request& request, httplib::Response& response);
        void HandleDeleteSession(const httplib::Request& request, httplib::Response& response);
        void HandleGetHistoryMessages(const httplib::Request& request, httplib::Response& response);
        void HandleSendMessages(const httplib::Request& request, httplib::Response& response);
        void HandleSendMessagesAsStream(const httplib::Request& request, httplib::Response& response);
        //用户管理
        void HandleRegister(const httplib::Request& request, httplib::Response& response);
    private:
        std::shared_ptr<Cplusplus_LLM_Provider::ChatSDK> _chatsdk ;
        std::shared_ptr<httplib::Server> _chatserver ;
        bool _isrunning ;
    };
}