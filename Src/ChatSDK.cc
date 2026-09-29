#include "../Include/ChatSDK.h"
#include "../Include/Util/LogModule.h"
#include "../Include/Common.h"
#include "../Include/CommonStruct.h"
#include "../Include/SessionManager.h"
#include "../Include/LLMManager.h"
#include "../Include/ChatGPTProvider.h"
#include "../Include/DeepSeekProvider.h"
#include "../Include/KimiProvider.h"
#include "../Include/OllamaProvider.h"
namespace Cplusplus_LLM_Provider
{
    void ChatSDK::registerAllProvider(const std::vector<std::shared_ptr<Config>>& configs)
    {
        if(!_llmManager.IsThisModelAvailable("deepseek-flash"))
        {
            auto deepProvider = std::make_unique<DeepSeekProvider>();
            _llmManager.RegistrModule("deepseek-flash",move(deepProvider));
            LogModule::INFO("{}模型注册成功","deepseek-flash");
        }
        if(!_llmManager.IsThisModelAvailable("gpt-5.5"))
        {
            auto gptProvider = std::make_unique<ChatGPTProvider>();
            _llmManager.RegistrModule("gpt-5.5",move(gptProvider));
            LogModule::INFO("{}模型注册成功","gpt-5.5");
        }
        if(!_llmManager.IsThisModelAvailable("kimi-k2.6"))
        {
            auto kimiProvider = std::make_unique<KimiProvider>();
            _llmManager.RegistrModule("kimi-k2.6",move(kimiProvider));
            LogModule::INFO("{}模型注册成功","kimi-k2.6");
        }
        for(auto conf : configs)
        {
            auto Config = std::dynamic_pointer_cast<OllamaConfig>(conf);
            std::string ModelName = Config->_modelName ;
            if(!_llmManager.IsThisModelAvailable(ModelName))
            {
                auto ollamaProvider = std::make_unique<OllamaProvider>();
                _llmManager.RegistrModule(ModelName,move(ollamaProvider));
                LogModule::INFO("{}模型注册成功",ModelName);
            }
        }
    }


    bool ChatSDK::initModels(const std::vector<std::shared_ptr<Config>>& configs)
    {
        registerAllProvider(configs);
        initProviders(configs);
        _initialized = true;
        return true;
    }


    void ChatSDK::initProviders(const std::vector<std::shared_ptr<Config>>& configs)
    {

    }
    bool ChatSDK::initAPIModelProviders(const std::string& modelName, const std::shared_ptr<APIConfig>& apiConfig)
    {

    }
    bool ChatSDK::initOllamaModelProviders(const std::string& modelName, const std::shared_ptr<OllamaConfig>& config)
    {

    }



    std::string ChatSDK::createSession(const std::string& modelName)
    {

    }
    std::shared_ptr<Session> ChatSDK::getSession(const std::string& sessionId)
    {

    }
    std::vector<std::string> ChatSDK::getSessionLists() const
    {

    }
    bool ChatSDK::deleteSession(const std::string& sessionId)
    {

    }
    std::vector<ModelInfo> ChatSDK::getAvailableModels() const
    {

    }
    std::string ChatSDK::sendMessage(const std::string& sessionId, const std::string& message)
    {

    }
    std::string ChatSDK::sendMessageStream(const std::string& sessionId, const std::string& message, 
        std::function<void(const std::string&, bool)> callback)
    {
        
    }
}
