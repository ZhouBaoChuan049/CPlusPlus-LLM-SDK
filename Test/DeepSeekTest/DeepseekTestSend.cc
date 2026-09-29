#include <iostream>
#include "../../Include/Util/LogModule.h"
#include "../../Include/DeepSeekProvider.h"
#include "../../Include/LLMProvider.h"
#include "../../Include/CommonStruct.h"
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
    std::cout<<provider->GetModelName()<<std::endl;
    std::cout<<provider->GetModelDescription()._ModelDesc<<std::endl;

    std::vector<Message> messages;
    //messages.push_back({"user" , "你好!请告诉我你的名字!"});
    messages.push_back({"user" , "你好!请介绍你自己!"}); 
    
    std::unordered_map<std::string,std::string> RequestPrograms;
    RequestPrograms["temperature"] = "1.2";
    RequestPrograms["Max_token"] = "512";
    RequestPrograms["model"] = "deepseek-flash";

    std::string response = provider->SendMessages(messages , RequestPrograms);
    ASSERT_FALSE(response.empty());
    //std::cout<<response<<std::endl;
    LogModule::INFO("Response is{}.",response);
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