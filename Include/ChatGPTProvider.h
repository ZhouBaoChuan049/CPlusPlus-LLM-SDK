#ifndef __CHATGPT_PROVIDER__
#define __CHATGPT_PROVIDER__
#include "Common.h"
#include "./Util/LogModule.h"
#include "CommonStruct.h"
#include "LLMProvider.h"
namespace Cplusplus_LLM_Provider
{
    class ChatGPTProvider : public LLMProvider  
    {
    public:
        ~ChatGPTProvider()override = default  ;
        ChatGPTProvider() = default ;
        void InitModel(std::shared_ptr<Config> Config) override ;
        
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
        bool IsModelAvailable() override ;
    };
}





#endif
