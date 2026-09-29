#ifndef __COMMON_STRUCT__
#define __COMMON_STRUCT__
#include "Common.h"

namespace Cplusplus_LLM_Provider
{
    class Message
    {
    public:
        std::string _MessageID ;
        std::string _SessionID ;
        std::string _Role ;
        std::string _Content ;
        std::time_t _Time ; 
        Message(const std::string& role , const std::string& content)
            :_Role(role),
            _Content(content)
        {}
    };
    //LLM介绍信息
    class ModelInfo
    {
    public:
        std::string _ModelName ;
        std::string _ModelDesc ;
        std::string _APIAccessAddress ; 
        bool _IsThisModelAvailable ;
        ModelInfo(
            const std::string& modelname = "", 
            const std::string modeldesc = "",
            const std::string& address = ""
        )
            :_ModelName(modelname),
             _ModelDesc(modeldesc),
             _APIAccessAddress(address)
        {}
    };
    struct Config{
        virtual ~Config() = default;
        std::string _modelName;       
        double _temperature = 0.7;    
        int _maxTokens = 2048;       
    };

    struct APIConfig : public Config{
        std::string _apiKey; 
    };

    struct OllamaConfig : public Config{
        std::string _modelDesc;    
        std::string _endpoint; 
        //因为Ollama模型他可以部署在很多地方，所以这个endpoint必须有外部主动提供。    
    };

    class Session
    {
    public:
        std::string _SessionID ;
        std::string _ModelNameUsed ;
        std::vector<Message> _Messages;
        std::time_t _TimeCreate ;
        std::time_t _LastTime ;
        Session(std::string name)
            :_ModelNameUsed(name)
        {}
    };
}

#endif