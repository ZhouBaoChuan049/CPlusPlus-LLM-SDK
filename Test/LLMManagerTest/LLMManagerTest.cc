#include <iostream>
#include "../../ChatSDK/Include/Util/LogModule.h"
#include "../../ChatSDK/Include/LLMManager.h"
#include "../../ChatSDK/Include/LLMProvider.h"
#include "../../ChatSDK/Include/DeepSeekProvider.h"
#include "../../ChatSDK/Include/ChatGPTProvider.h"
#include "../../ChatSDK/Include/OllamaProvider.h"
#include "../../ChatSDK/Include/CommonStruct.h"
#include <gtest/gtest.h>
using namespace Cplusplus_LLM_Provider ;

    // class ModelInfo
    // {
    // public:
    //     std::string _ModelName ;
    //     std::string _ModelDesc ;
    //     std::string _ModelProvider ;
    //     std::string _APIAccessAddress ; 
    //     bool _IsThisModelAvailable ;
    //     ModelInfo(const std::string& modelname = "", const std::string modeldesc = "",
    //         const std::string& provider = "",const std::string& address = "")
    //         :_ModelName(modelname),
    //          _ModelDesc(modeldesc),
    //          _ModelProvider(provider),
    //          _APIAccessAddress(address)
    //     {}
    // };


TEST(TestLLMManager , TestFunctions)
{
    LLMManager manager ;
    manager.RegistrModule(
        "deepseek-flash",
        std::make_shared<DeepSeekProvider>()
    );
    manager.RegistrModule(
        "gpt-5.5",
        std::make_shared<ChatGPTProvider>()
    );
    manager.RegistrModule(
        "deepseek-r1:1.5b",
        std::make_shared<OllamaProvider>()
    );
    
    auto ConfigForDeepseek = std::make_shared<APIConfig>();
    ConfigForDeepseek->_apiKey = getenv("deepseek_apikey") ;
    manager.InitThisModule("deepseek-flash", ConfigForDeepseek);

    auto ConfigForGpt = std::make_shared<APIConfig>();
    ConfigForGpt->_apiKey = getenv("chatgpt_apikey") ;
    manager.InitThisModule("gpt-5.5", ConfigForGpt);
    
    auto ConfigForOllama = std::make_shared<OllamaConfig>();
    ConfigForOllama->_modelName = "deepseek-r1:1.5b" ;
    ConfigForOllama->_modelDesc = "deepseek-r1:1.5b 是 DeepSeek-R1 \
    系列中参数最小的蒸馏模型,基于 Qwen2.5-1.5B 微调而来。它保留了 \
    R1 的推理能力,体积仅约1.1GB,普通电脑就能流畅运行。在数学和编\
    程任务上表现不错,MIT 协议允许免费商用";
    ConfigForOllama->_endpoint = "127.0.0.1:11434" ;
    manager.InitThisModule("deepseek-r1:1.5b", ConfigForOllama);
    
    std::vector<std::pair<std::string,ModelInfo>> models = manager.GetAllAvailableModule();
    for(auto m : models)
        std::cout<<"模型名称:["<<m.first<<"],模型描述信息:["<<m.second._ModelDesc<<"]"<<std::endl;
    
    ASSERT_TRUE(manager.IsThisModelAvailable("deepseek-flash"));
    ASSERT_TRUE(manager.IsThisModelAvailable("gpt-5.5"));
    ASSERT_TRUE(manager.IsThisModelAvailable("deepseek-r1:1.5b"));

#ifdef __DeepSeek_Send__
    std::vector<CppAiChatSdk::Message> messages;
    messages.push_back({"user" , "你好!请介绍你自己!"}); 
    
    std::unordered_map<std::string,std::string> RequestPrograms;
    RequestPrograms["temperature"] = "1.2";
    RequestPrograms["Max_token"] = "40960";

    manager.SendMessageToThisModlue(
        "deepseek-flash",
        messages,
        RequestPrograms
    );
#ifdef __STREAM__
    std::string AllResponse = manager.SendMessageToThisModlueAsStream(
        "deepseek-flash",
        messages, 
        RequestPrograms,
        [](std::string content , bool least){
            if(least)
                return ;
            LogModule::INFO(content);
        }
    );
    ASSERT_FALSE(AllResponse.empty());
    LogModule::INFO("Response is{}.",AllResponse);
#endif
#endif

}

int main(int argc,char* argv[])
{
    ::testing::InitGoogleTest(&argc,argv);
    LogModule::SpdLogPack::SpdLogInit(
        "LLMTestLog", 
        "log.dat",
        spdlog::level::info,
        0
    );
    return RUN_ALL_TESTS();
}