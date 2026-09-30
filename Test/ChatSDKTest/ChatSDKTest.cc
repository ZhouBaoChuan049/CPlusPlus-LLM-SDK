#include <iostream>
#include <gtest/gtest.h>
#include "../../Include/Util/LogModule.h"
#include "../../Include/CommonStruct.h"
#include "../../Include/ChatSDK.h"

using namespace Cplusplus_LLM_Provider ;
TEST(ChatSDKTEST, SendMessges)
{
    std::shared_ptr<ChatSDK> chatsdk = std::make_shared<ChatSDK>();
    std::vector<std::shared_ptr<Config>> SDKConfigs ;
    //配置参数://////////////////
    std::shared_ptr<APIConfig> DeepseekConfig = std::make_shared<APIConfig>();
    DeepseekConfig->_maxTokens = 4096;
    DeepseekConfig->_temperature = 1.0;
    DeepseekConfig->_modelName = "deepseek-flash";
    DeepseekConfig->_apiKey = getenv("deepseek_apikey") ;
    SDKConfigs.push_back(DeepseekConfig);

    std::shared_ptr<APIConfig> GPTConfig = std::make_shared<APIConfig>();
    GPTConfig->_maxTokens = 4096;
    GPTConfig->_temperature = 1.0;
    GPTConfig->_modelName = "gpt-5.5";
    GPTConfig->_apiKey = getenv("chatgpt_apikey") ;
    SDKConfigs.push_back(GPTConfig);

    std::shared_ptr<APIConfig> KimiConfig = std::make_shared<APIConfig>();
    KimiConfig->_maxTokens = 4096;
    KimiConfig->_temperature = 1.0;
    KimiConfig->_modelName = "kimi-k2.6";
    KimiConfig->_apiKey = getenv("KIMI_API_KEY");
    SDKConfigs.push_back(KimiConfig);

    std::shared_ptr<OllamaConfig> DeepseekR1Config = std::make_shared<OllamaConfig>();
    DeepseekR1Config->_temperature = 1.0;
    DeepseekR1Config->_modelName = "deepseek-r1:1.5b";
    DeepseekR1Config->_modelDesc = "deepseek-r1:1.5b 是 DeepSeek-R1 \
    系列中参数最小的蒸馏模型，基于 Qwen2.5-1.5B 微调而来。它保留了 \
     R1 的推理能力，体积仅约 1.1GB,普通电脑就能流畅运行。在数学和编\
     程任务上表现不错,MIT 协议允许免费商用" ;
    DeepseekR1Config->_endpoint = "127.0.0.1:11434" ;
    DeepseekR1Config->_maxTokens = 4096 ;
    DeepseekR1Config->Num_Ctx = 2048 ;
    SDKConfigs.push_back(DeepseekR1Config);
    ////////////////////////////

    bool isInitModelok = chatsdk->initModels(SDKConfigs);
    ASSERT_TRUE(isInitModelok);
    std::string sessionid = chatsdk->createSession("新建会话-1","deepseek-flash");
    std::string Resp = chatsdk->sendMessage(sessionid, "你好!请介绍你自己!");
    std::cout<<Resp<<std::endl;

    std::string RespStream = chatsdk->sendMessageStream(
        sessionid, 
        "你好!请介绍你自己!",
        [](std::string response, bool check){
            if(check == true)
                return;
            LogModule::INFO("NewInfo:[{}]",response);
    });
    std::cout<<RespStream<<std::endl;
}

int main(int argc,char* argv[])
{
    ::testing::InitGoogleTest(&argc,argv);
    LogModule::SpdLogPack::SpdLogInit("GoogleTestLog",
                          "TestLogFile.dat",
                           spdlog::level::info,
                           CONSOLE__MODE);
    return RUN_ALL_TESTS() ;
}