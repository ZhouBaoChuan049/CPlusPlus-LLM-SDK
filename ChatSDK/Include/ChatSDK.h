#ifndef __CHAT_SDK__
#define __CHAT_SDK__
#include "Common.h"
#include "CommonStruct.h"
#include "SessionManager.h"
#include "LLMManager.h"
#include "UserManager.h"
namespace Cplusplus_LLM_Provider
{
    class ChatSDK
    {
    public:
        explicit ChatSDK(const std::string& dbname = "chat_sdk.db")
            : _sessionManager(dbname),
              _userManager(dbname)
        {}
        bool initModels(const std::vector<std::shared_ptr<Config>>& configs);
        std::string createSession(const std::string SessionName, const std::string& modelName, const std::string userName = "");
        std::shared_ptr<Session> getSession(const std::string& sessionId);
        std::vector<std::string> getSessionLists() const;
        bool deleteSession(const std::string& sessionId);
        std::vector<std::pair<std::string,ModelInfo>> getAvailableModels() ;
        std::string sendMessage(const std::string& sessionId, const std::string& message);
        std::string sendMessageStream(const std::string& sessionId, const std::string& message, 
                                            std::function<void(const std::string&, bool)> callback);   
    private:
        void registerAllProvider(const std::vector<std::shared_ptr<Config>>& configs);
        void initProviders(const std::vector<std::shared_ptr<Config>>& configs);
        bool initAPIModelProviders(const std::string& modelName, const std::shared_ptr<APIConfig>& apiConfig);
        bool initOllamaModelProviders(const std::string& modelName, const std::shared_ptr<OllamaConfig>& ollamaconfig);

    private:
        bool _initialized = false;        
        std::unordered_map<std::string, std::shared_ptr<Config>> _modelConfigs;  // 模型配置
        LLMManager _llmManager;           
    public:
        SessionManager _sessionManager;  
        UserManager _userManager;  
    
    };
}

#endif