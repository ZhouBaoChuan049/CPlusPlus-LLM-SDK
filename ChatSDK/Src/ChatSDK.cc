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
        std::unordered_set<std::string> Check ;
        for(auto conf : configs)
        {
            auto Config = std::dynamic_pointer_cast<OllamaConfig>(conf);
            if(Config!= nullptr)
            {
                std::string ModelName = Config->_modelName ;
                if(Check.find(ModelName) == Check.end())
                {
                    Check.insert(ModelName);//去重机制
                    if(!_llmManager.IsThisModelAvailable(ModelName))
                    {
                        auto ollamaProvider = std::make_unique<OllamaProvider>();
                        _llmManager.RegistrModule(ModelName,move(ollamaProvider));
                        LogModule::INFO("{}模型注册成功",ModelName);
                    }
                }
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
        for(auto Conf : configs){
            std::unordered_set<std::string> Check ;//还是去重操作
            if(auto Config = std::dynamic_pointer_cast<APIConfig>(Conf))
            {
                std::string ModelName = Config->_modelName;
                if(Check.find(ModelName) == Check.end())
                {
                    Check.insert(ModelName);
                    if(ModelName == "deepseek-flash" ||
                        ModelName == "gpt-5.5" || ModelName == "kimi-k2.6"){
                            initAPIModelProviders(ModelName , Config);
                    }else{
                        LogModule::ERROR("对不起!你要求的模型我们暂时不支持,敬请期待!");
                    }
                }
            }
            else if(auto Config = std::dynamic_pointer_cast<OllamaConfig>(Conf))
            {
                std::string ModelName = Config->_modelName;
                if(Check.find(ModelName) == Check.end())
                {
                    Check.insert(ModelName);
                    if(ModelName == "deepseek-r1:1.5b"){
                            initOllamaModelProviders(ModelName , Config);
                    }else{
                        LogModule::ERROR("对不起!你要求的模型我们暂时不支持,敬请期待!");
                    }
                }
            }
        }
    }
    bool ChatSDK::initAPIModelProviders(const std::string& modelName, const std::shared_ptr<APIConfig>& apiConfig)
    {
        if(modelName == ""){
            LogModule::ERROR("参数不太对,模型名称未填写!");
            return false ;
        }
        if(apiConfig == nullptr || apiConfig->_apiKey == ""){
            LogModule::ERROR("参数错误,模型配置参数为空或APIkey未填写");
            return false ;
        }
        if(_modelConfigs.find(modelName) != _modelConfigs.end()){
            LogModule::INFO("模型已就绪!不需要再次初始化.");
            return true ;
        }
        if(!_llmManager.InitThisModule(modelName , apiConfig))
        {
            LogModule::ERROR("模型{}初始化失败!",modelName);
            return false ;
        }
        _modelConfigs[modelName] = apiConfig ;
        LogModule::INFO("模型{}初始化成功!",modelName);
        return true ;
    }
    bool ChatSDK::initOllamaModelProviders(const std::string& modelName, const std::shared_ptr<OllamaConfig>& ollamaconfig)
    {
        if(modelName == ""){
            LogModule::ERROR("参数不太对,模型名称未填写!");
            return false ;
        }
        if(ollamaconfig == nullptr ){
            LogModule::ERROR("参数错误,模型配置参数为空");
            return false ;
        }
        if(_modelConfigs.find(modelName) != _modelConfigs.end()){
            LogModule::INFO("模型已就绪!不需要再次初始化.");
            return true ;
        }
        if(!_llmManager.InitThisModule(modelName , ollamaconfig))
        {
            LogModule::ERROR("模型{}初始化失败!",modelName);
            return false ;
        }
        _modelConfigs[modelName] = ollamaconfig ;
        LogModule::INFO("模型{}初始化成功!",modelName);
        return true ;
    }
    std::string ChatSDK::createSession(const std::string SessionName, const std::string& modelName, const std::string userName)
    {
        if(_initialized == false)
        {
            LogModule::ERROR("错误!模型未初始化.");
            return "" ;
        }
        std::string sessionid = _sessionManager.CreatSession(SessionName, modelName, userName);
        return sessionid ;//这里应该不会出问题
    }
    std::shared_ptr<Session> ChatSDK::getSession(const std::string& sessionId)
    {
        if(_initialized == false)
        {
            LogModule::ERROR("错误!模型未初始化.");
            return nullptr ;
        }
        std::shared_ptr<Session> sessionptr = _sessionManager.GetSession(sessionId);
        if(sessionptr == nullptr)
        {
            LogModule::ERROR("获取会话失败!");
            return nullptr ;
        }
        return sessionptr ;
    }
    std::vector<std::string> ChatSDK::getSessionLists() const
    {
        if(_initialized == false)
        {
            LogModule::ERROR("错误!模型未初始化.");
            return {} ;
        }
        return _sessionManager.GetSessionLists();
    }
    bool ChatSDK::deleteSession(const std::string& sessionId)
    {
        if(_initialized == false)
        {
            LogModule::ERROR("错误!模型未初始化.");
            return false;
        }
        if(_sessionManager.DeleteSession(sessionId) == false)
        {
            LogModule::ERROR("删除会话{}失败!",sessionId);
            return false ;
        }
        LogModule::INFO("删除会话{}成功!",sessionId);
        return true ;
    }
    std::vector<std::pair<std::string,ModelInfo>> ChatSDK::getAvailableModels()
    {
        std::vector<std::pair<std::string,ModelInfo>> ret = 
            _llmManager.GetAllAvailableModule();
        return ret;
    }
    std::string ChatSDK::sendMessage(const std::string& sessionId, const std::string& message)
    {
        //我要告诉大模型的消息message
        std::shared_ptr<Session> Sessptr = 
            _sessionManager.GetSession(sessionId);
        if(Sessptr == nullptr)
        {
            LogModule::ERROR("获取会话失败!");
            return "" ;
        }
        
        Message NewMessage_1("user",message);
        _sessionManager.AddMessage(sessionId, NewMessage_1);
        
        std::string SessModelName = Sessptr->_ModelNameUsed;
        std::vector<Message> HistoryMessages = 
            _sessionManager.GetHistoryMessages(sessionId);
        std::unordered_map<std::string,std::string> RequestPrograms ;
        auto it = _modelConfigs.find(SessModelName);//配置信息的智能指针
        if(it == _modelConfigs.end())
        {
            LogModule::ERROR("找不到模型{}的配置信息!",SessModelName);
            return "" ;
        }
        RequestPrograms["Max_token"] = std::to_string(it->second->_maxTokens);
        RequestPrograms["temperature"] = std::to_string(it->second->_temperature);
        //特别判断:Ollama的一个专门配置参数
        auto _OllamaConfig = 
            std::dynamic_pointer_cast<OllamaConfig>(it->second);
        if(_OllamaConfig != nullptr)
            RequestPrograms["Num_Ctx"] = std::to_string(_OllamaConfig->Num_Ctx);
        std::string AssistantResponse = _llmManager.SendMessageToThisModlue(
            SessModelName,
            HistoryMessages,
            RequestPrograms
        );
        Message NewMessage_2("assistant",AssistantResponse);
        _sessionManager.AddMessage(sessionId, NewMessage_2);
        return AssistantResponse ;
    }
    std::string ChatSDK::sendMessageStream(const std::string& sessionId, const std::string& message, 
        std::function<void(const std::string&, bool)> callback)
    {
        std::shared_ptr<Session> Sessptr = 
            _sessionManager.GetSession(sessionId);
        if(Sessptr == nullptr)
        {
            LogModule::ERROR("获取会话失败!");
            return "" ;
        }
        
        Message NewMessage_1("user",message);
        _sessionManager.AddMessage(sessionId, NewMessage_1);
        
        std::string SessModelName = Sessptr->_ModelNameUsed;
        std::vector<Message> HistoryMessages = 
            _sessionManager.GetHistoryMessages(sessionId);
        std::unordered_map<std::string,std::string> RequestPrograms ;
        auto it = _modelConfigs.find(SessModelName);//配置信息的智能指针
        if(it == _modelConfigs.end())
        {
            LogModule::ERROR("找不到模型{}的配置信息!",SessModelName);
            return "" ;
        }
        RequestPrograms["Max_token"] = std::to_string(it->second->_maxTokens);
        RequestPrograms["temperature"] = std::to_string(it->second->_temperature);
        //特别判断:Ollama的一个专门配置参数
        auto _OllamaConfig = 
            std::dynamic_pointer_cast<OllamaConfig>(it->second);
        if(_OllamaConfig != nullptr)
            RequestPrograms["Num_Ctx"] = std::to_string(_OllamaConfig->Num_Ctx);
        std::string AssistantResponse = _llmManager.SendMessageToThisModlueAsStream(
            SessModelName,
            HistoryMessages,
            RequestPrograms,
            callback
        );
        Message NewMessage_2("assistant",AssistantResponse);
        _sessionManager.AddMessage(sessionId, NewMessage_2);
        return AssistantResponse ;
    }
}