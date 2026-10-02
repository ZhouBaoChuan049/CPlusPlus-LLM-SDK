#include <iostream>

#include "../../ChatSDK/Include/Util/LogModule.h"
#include "../../ChatSDK/Include/KimiProvider.h"
#include "../../ChatSDK/Include/LLMProvider.h"
#include "../../ChatSDK/Include/CommonStruct.h"

#include <gtest/gtest.h>

using namespace Cplusplus_LLM_Provider;

// #define __SEND_MESSAGE__

#ifdef __SEND_MESSAGE__

TEST(KimiProviderTest, SendMessageTest)
{
    std::unique_ptr<Cplusplus_LLM_Provider::LLMProvider> provider =
        std::make_unique<Cplusplus_LLM_Provider::KimiProvider>();

    ASSERT_FALSE(provider == nullptr);

    auto Config = std::make_shared<APIConfig>();

    Config->_apiKey = getenv("KIMI_API_KEY");

    provider->InitModel(Config);

    ASSERT_TRUE(provider->IsModelAvailable());

    std::cout << provider->GetModelName() << std::endl;

    std::cout << provider->GetModelDescription()._ModelDesc
              << std::endl;

    std::vector<Message> messages;

    messages.push_back(
        {"user", "你好!请介绍你自己!"}
    );

    std::unordered_map<std::string, std::string> RequestPrograms;

    RequestPrograms["Max_token"] = "2048";

    std::string response =
        provider->SendMessages(
            messages,
            RequestPrograms
        );

    ASSERT_FALSE(response.empty());

    LogModule::INFO("Response is{}.", response);
}

#endif


#define __SEND_MESSAGE_STREAM__

#ifdef __SEND_MESSAGE_STREAM__

TEST(KimiProviderTest, SendMessageStreamTest)
{
    std::unique_ptr<Cplusplus_LLM_Provider::LLMProvider> provider =
        std::make_unique<Cplusplus_LLM_Provider::KimiProvider>();

    ASSERT_FALSE(provider == nullptr);

    auto Config = std::make_shared<APIConfig>();

    Config->_apiKey = getenv("KIMI_API_KEY");

    provider->InitModel(Config);

    ASSERT_TRUE(provider->IsModelAvailable());

    std::cout << provider->GetModelName() << std::endl;

    std::cout << provider->GetModelDescription()._ModelDesc
              << std::endl;

    std::vector<Message> messages;

    messages.push_back(
        {"user", "你好!请介绍你自己!"}
    );

    std::unordered_map<std::string, std::string> RequestPrograms;

    RequestPrograms["Max_token"] = "4096";

    std::string AllResponse =
        provider->SendMessagesAsStream(
            messages,
            RequestPrograms,
            [](std::string content, bool least)
            {
                if (least)
                    return;
                LogModule::INFO(content);
            }
        );

    ASSERT_FALSE(AllResponse.empty());

    LogModule::INFO(
        "Response is{}.",
        AllResponse
    );
}

#endif


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    LogModule::SpdLogPack::SpdLogInit("GoogleTestLog",
                          "TestLogFile.dat",
                           spdlog::level::info,
                           CONSOLE__MODE);

    return RUN_ALL_TESTS();
}