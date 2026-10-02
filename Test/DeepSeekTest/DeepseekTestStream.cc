#include <iostream>
#include "../../ChatSDK/Include/Util/LogModule.h"
#include "../../ChatSDK/Include/DeepSeekProvider.h"
#include "../../ChatSDK/Include/LLMProvider.h"
#include "../../ChatSDK/Include/CommonStruct.h"
#include <gtest/gtest.h>
using namespace Cplusplus_LLM_Provider ;

TEST(DeepSeekProviderTest , SendMessageTest)
{
    std::unique_ptr<Cplusplus_LLM_Provider::LLMProvider> provider =
        std::make_unique<Cplusplus_LLM_Provider::DeepSeekProvider>();

    ASSERT_FALSE(provider == nullptr);
    
    auto Config = std::make_shared<APIConfig>();
    Config->_modelName = "deepseek-flash";
    Config->_apiKey = getenv("deepseek_apikey") ;
    provider->InitModel(Config);

    ASSERT_TRUE(provider->IsModelAvailable());

    std::vector<Message> messages;
    messages.push_back({"user" , "你好!请介绍你自己!"}); 

    std::unordered_map<std::string,std::string> RequestPrograms;
    RequestPrograms["temperature"] = "1.2";
    RequestPrograms["Max_token"] = "4096";
    RequestPrograms["model"] = "deepseek-flash";

    // std::string SendMessagesAsStream(std::vector<CppAiChatSdk::Message>& messages,
    //     std::unordered_map<std::string, std::string>& RequestPrograms,
    //     func_t callback)override;   
    //using func_t = std::function<void(std::string , bool)>;
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
                           spdlog::level::debug,
                           CONSOLE__MODE);
    return RUN_ALL_TESTS();
}