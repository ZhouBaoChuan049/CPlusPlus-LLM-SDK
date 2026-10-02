#include <iostream>
#include "../../ChatSDK/Include/Util/LogModule.h"
#include "../../ChatSDK/Include/ChatGPTProvider.h"
#include "../../ChatSDK/Include/LLMProvider.h"
#include "../../ChatSDK/Include/CommonStruct.h"
#include <gtest/gtest.h>

using namespace Cplusplus_LLM_Provider ;
//#define __SEND_MESSAGE__ 
#ifdef __SEND_MESSAGE__ 

TEST(ChatGptProviderTest , SendMessageTest)
{
    std::unique_ptr<Cplusplus_LLM_Provider::LLMProvider> provider =
         std::make_unique<Cplusplus_LLM_Provider::ChatGPTProvider>();

    ASSERT_FALSE(provider == nullptr);
    
    auto Config = std::make_shared<APIConfig>();
    Config->_apiKey = getenv("chatgpt_apikey") ;
    provider->InitModel(Config);

    ASSERT_TRUE(provider->IsModelAvailable());
    std::cout<<provider->GetModelName()<<std::endl;
    std::cout<<provider->GetModelDescription()._ModelDesc<<std::endl;

    std::vector<Message> messages;
    //messages.push_back({"user" , "你好!请告诉我你的名字!"});
    messages.push_back({"user" , "你好!请介绍你自己!"}); 
    
    std::unordered_map<std::string,std::string> RequestPrograms;
    RequestPrograms["temperature"] = "1";
    RequestPrograms["max_output_tokens"] = "2048";
    //RequestPrograms["model"] = "gpt-5.5";//我忽然觉得不应该把模型的选择交给外部管理

    std::string response = provider->SendMessages(messages , RequestPrograms);
    ASSERT_FALSE(response.empty());
    //std::cout<<response<<std::endl;
    LogModule::INFO("Response is{}.",response);
}   
#endif

#define __SEND_MESSAGE_STREAM__ 
#ifdef __SEND_MESSAGE_STREAM__ 
TEST(ChatGptProviderTest , SendMessageTest)
{
    std::unique_ptr<Cplusplus_LLM_Provider::LLMProvider> provider =
         std::make_unique<Cplusplus_LLM_Provider::ChatGPTProvider>();

    ASSERT_FALSE(provider == nullptr);
    
    auto Config = std::make_shared<APIConfig>();
    Config->_apiKey = getenv("chatgpt_apikey") ;
    provider->InitModel(Config);

    ASSERT_TRUE(provider->IsModelAvailable());
    std::cout<<provider->GetModelName()<<std::endl;
    std::cout<<provider->GetModelDescription()._ModelDesc<<std::endl;

    std::vector<Message> messages;
    messages.push_back({"user" , "你好!请介绍你自己!"}); 

    std::unordered_map<std::string,std::string> RequestPrograms;
    RequestPrograms["temperature"] = "1";
    RequestPrograms["max_output_tokens"] = "4096";
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
#endif

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc , argv);
    LogModule::SpdLogPack::SpdLogInit("GoogleTestLog",
                          "TestLogFile.dat",
                           spdlog::level::info,
                           CONSOLE__MODE);
    return RUN_ALL_TESTS();
}