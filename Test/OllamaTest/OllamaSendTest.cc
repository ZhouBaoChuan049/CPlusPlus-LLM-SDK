#include <iostream>
#include "../../Include/Util/LogModule.h"
#include "../../Include/OllamaProvider.h"
#include "../../Include/LLMProvider.h"
#include "../../Include/CommonStruct.h"
#include <gtest/gtest.h>
using namespace Cplusplus_LLM_Provider ;
#ifdef __OLLAMA_SEND_MESSAGE_TEST__
TEST(OllamaProviderTest , SendMessageTest)
{
    std::unique_ptr<Cplusplus_LLM_Provider::LLMProvider> provider =
         std::make_unique<Cplusplus_LLM_Provider::OllamaProvider>();

    ASSERT_FALSE(provider == nullptr);
    
    auto Config = std::make_shared<OllamaConfig>();
    Config->_modelName = "deepseek-r1:1.5b" ;
    Config->_modelDesc = "deepseek-r1:1.5b 是 DeepSeek-R1 \
    系列中参数最小的蒸馏模型，基于 Qwen2.5-1.5B 微调而来。它保留了 \
     R1 的推理能力，体积仅约 1.1GB,普通电脑就能流畅运行。在数学和编\
     程任务上表现不错,MIT 协议允许免费商用" ;
    Config->_endpoint = "127.0.0.1:11434" ;
    provider->InitModel(Config);

    ASSERT_TRUE(provider->IsModelAvailable());
    std::cout<<provider->GetModelName()<<std::endl;
    std::cout<<provider->GetModelDescription()._ModelDesc<<std::endl;

    std::vector<Message> messages;
    //messages.push_back({"user" , "你好!请告诉我你的名字!"});
    messages.push_back({"user", "请生成十个字！什么都行。"});
    
    std::unordered_map<std::string,std::string> RequestPrograms;
    RequestPrograms["temperature"] = "1.2";
    RequestPrograms["Max_token"] = "10";
    RequestPrograms["Num_Ctx"] = "2048" ;
    std::string response = provider->SendMessages(messages , RequestPrograms);
    ASSERT_FALSE(response.empty());
    //std::cout<<response<<std::endl;
    LogModule::INFO("Response is{}.",response);
}   
#endif 

TEST(OllamaProviderTest , SendMessageTest)
{
    std::unique_ptr<Cplusplus_LLM_Provider::LLMProvider> provider =
         std::make_unique<Cplusplus_LLM_Provider::OllamaProvider>();

    ASSERT_FALSE(provider == nullptr);
    
    auto Config = std::make_shared<OllamaConfig>();
    Config->_modelName = "deepseek-r1:1.5b" ;
    Config->_modelDesc = "deepseek-r1:1.5b 是 DeepSeek-R1 \
    系列中参数最小的蒸馏模型，基于 Qwen2.5-1.5B 微调而来。它保留了 \
     R1 的推理能力，体积仅约 1.1GB,普通电脑就能流畅运行。在数学和编\
     程任务上表现不错,MIT 协议允许免费商用" ;
    Config->_endpoint = "127.0.0.1:11434" ;
    provider->InitModel(Config);

    ASSERT_TRUE(provider->IsModelAvailable());
    std::cout<<provider->GetModelName()<<std::endl;
    std::cout<<provider->GetModelDescription()._ModelDesc<<std::endl;

    std::vector<Message> messages;
    messages.push_back({"user" , "你好!请介绍你自己!"}); 

    std::unordered_map<std::string,std::string> RequestPrograms;
    RequestPrograms["temperature"] = "1.2";
    RequestPrograms["Max_token"] = "10";
    RequestPrograms["Num_Ctx"] = "2048" ;

    std::string AllResponse = provider->SendMessagesAsStream(
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
}   

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc , argv);
    LogModule::SpdLogPack::SpdLogInit("GoogleTestLog",
                          "TestLogFile.dat",
                           spdlog::level::info,
                           CONSOLE__MODE);
    return RUN_ALL_TESTS();
}