#ifndef __OLLAMA_PROVIDER__
#define __OLLAMA_PROVIDER__
#include "Common.h"
#include "CommonStruct.h"
#include "./Util/LogModule.h"
#include "LLMProvider.h"
namespace Cplusplus_LLM_Provider
{
    class OllamaProvider: public LLMProvider
    {
    public:
        OllamaProvider() = default ;
        ~OllamaProvider() override = default ;
        void InitModel(std::shared_ptr<Config> Config) override;

        std::string GetModelName() override;
        ModelInfo GetModelDescription() override;
        std::string SendMessages(
            std::vector<Message>& messages,
            std::unordered_map<std::string,std::string>& RequestPrograms
        ) override;
        std::string SendMessagesAsStream(
            std::vector<Message>& messages,
            std::unordered_map<std::string, std::string>& RequestPrograms,
            func_t callback
        ) override;
    private:
        bool IsModelAvailable() override;
        std::string Serialize(std::vector<Message>& messages,
             std::unordered_map<std::string,std::string>& RequestPrograms,bool isstream);
        httplib::Result SendRequestMessage(std::string& RequestBodyString);
        Json::Value Deserialize(std::string& ResponseString);
        httplib::Client CreateClient(int commect_timeout , int read_timeout);
        std::string AnalyseForSendMessage(Json::Value Response ,const std::string&& ResponseString);
        std::string AnalyseForSendMessageStream(Json::Value Response ,const std::string&& ResponseString);
    private:
        std::string _ModelName ;
        std::string _ModelDesc ;
    };
}

#endif