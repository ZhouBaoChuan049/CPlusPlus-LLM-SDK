#ifndef __LLMPROVIDER__
#define __LLMPROVIDER__
#include "Common.h"
#include "CommonStruct.h"

namespace Cplusplus_LLM_Provider
{
using func_t = std::function<void(std::string , bool)>;
const std::string POS = "\n\n";
    class LLMProvider
    {
    public:
        LLMProvider()
            :_IsAvailable(false),
             _ApiKey(""),
             _APIAccessAddress("")
        {}
        virtual ~LLMProvider() = default ;
        virtual void InitModel(std::shared_ptr<Config> Config) = 0;
        virtual bool IsModelAvailable() = 0;
        virtual std::string GetModelName() = 0;
        virtual ModelInfo GetModelDescription() = 0;
        virtual std::string SendMessages(std::vector<Message>& messages,
             std::unordered_map<std::string,std::string>& RequestPrograms) = 0;
        virtual std::string SendMessagesAsStream(std::vector<Message>& messages,
            std::unordered_map<std::string, std::string>& RequestPrograms,
            func_t callback) = 0;
    protected:
        void SetApiKey(std::string& ApiKey)
        {
            _ApiKey = ApiKey ;
        }
        void SetAPIAccessAddress(std::string APIAccessAddress)
        {
            _APIAccessAddress = APIAccessAddress ;
        }
        void SetAvailable(bool IsAvailable)
        {
            _IsAvailable = IsAvailable ;
        }
        std::string GetApiKey()
        {
            return _ApiKey ;
        }
        std::string GetAPIAccessAddress()
        {
            return _APIAccessAddress ;
        }
        bool GetAvailable()
        {
            return _IsAvailable ;
        }
    private:
        bool _IsAvailable ;
        std::string _ApiKey ;
        std::string _APIAccessAddress ;
    };
}

#endif