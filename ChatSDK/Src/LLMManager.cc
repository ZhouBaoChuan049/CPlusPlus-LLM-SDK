#include "../Include/Common.h"
#include "../Include/Util/LogModule.h"
#include "../Include/LLMManager.h"
namespace Cplusplus_LLM_Provider
{
    void LLMManager::SetModelInformation(std::string modelname,std::string info)
    {
        ProviderInfo[modelname]._ModelDesc = info ;
    }
    bool LLMManager::RegistrModule(
        std::string modelname, 
        std::shared_ptr<LLMProvider> provider
    ){
        if(!provider)
        {
            LogModule::ERROR("参数错误!The Provider is Nullptr");
            return false ;
        }
        if(ProviderCollections.find(modelname) == ProviderCollections.end())
            ProviderCollections.insert(make_pair(modelname, provider));
        if(ProviderInfo.find(modelname) ==ProviderInfo.end())   
            ProviderInfo.insert(make_pair(modelname, provider->GetModelDescription()));// 
        LogModule::INFO("Model:{}Have Been Registred",modelname);
        return true ;
    }
    bool LLMManager::InitThisModule(
        std::string modelname, 
        std::shared_ptr<Config> Config
    ){
        auto ollamaConfig = std::dynamic_pointer_cast<OllamaConfig>(Config);
        if(ollamaConfig)
            SetModelInformation(modelname, ollamaConfig->_modelDesc);
        if(ProviderCollections.find(modelname) == ProviderCollections.end())
        {
            LogModule::ERROR("模型未注册!不能初始化未注册的模型!");
            return false ;
        }
        ProviderCollections[modelname]->InitModel(Config);
        return true ;
    }
    std::vector<std::pair<std::string,ModelInfo>> LLMManager::GetAllAvailableModule()
    {
        std::vector<std::pair<std::string,ModelInfo>> ModelColl ;
        for(const auto& item : ProviderCollections)
        {
            ModelInfo info = item.second->GetModelDescription();
            if(info._IsThisModelAvailable)
                ModelColl.push_back(make_pair(item.first, info));
        }
        return ModelColl ;
    }
    bool LLMManager::IsThisModelAvailable(std::string modelname)
    {
        if(ProviderCollections.find(modelname) == ProviderCollections.end())
        {
            LogModule::ERROR("模型未注册!");
            return false ;
        }
        return ProviderCollections[modelname]->IsModelAvailable();
    }
    std::string LLMManager::SendMessageToThisModlue(
        std::string modelname,
        std::vector<Message> messages,
        std::unordered_map<std::string,std::string>& RequestPrograms)
    {
        if(ProviderCollections.find(modelname) == ProviderCollections.end()){
            LogModule::ERROR("模型未注册!");
            return "" ;
        }
        if(!IsThisModelAvailable(modelname)){
            LogModule::ERROR("模型未初始化");
            return "" ;
        }
        return ProviderCollections[modelname]->SendMessages(messages, RequestPrograms);
    }
    std::string LLMManager::SendMessageToThisModlueAsStream(
        std::string modelname,
        std::vector<Message> messages,
        std::unordered_map<std::string,std::string>& RequestPrograms,
        func_t callback)
    {
        if(ProviderCollections.find(modelname) == ProviderCollections.end()){
            LogModule::ERROR("模型未注册!");
            return "" ;
        }
        if(!IsThisModelAvailable(modelname)){
            LogModule::ERROR("模型未初始化");
            return "" ;
        }
        return ProviderCollections[modelname]->SendMessagesAsStream(messages, RequestPrograms,callback);
    }
}