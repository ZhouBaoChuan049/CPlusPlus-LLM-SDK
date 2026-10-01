#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <sstream>
#include <thread>

#include <jsoncpp/json/json.h>

#include "../../ChatServer/ChatServer.h"

using namespace ChatServerModule;

namespace
{
    // 测试用端口，避免与真实服务冲突
    constexpr int kServerPort = 18080;
    constexpr int kLifecyclePort = 18081;
    constexpr const char* kHost = "127.0.0.1";

    // 用一个"假但非空"的 key，保证构造函数里各 provider 能正常 InitModel，
    // 而不会因为空 key 触发 exit()；本测试不真正调用云端模型，只验证 HTTP 层。
    ServerConfig MakeConfig(int port)
    {
        ServerConfig c;
        c.host = kHost;
        c.port = port;
        c.deepseekAPIKey = "test-deepseek-key";
        c.chatGPTAPIKey = "test-gpt-key";
        c.geminiAPIKey = "test-kimi-key";
        c.ollamaModelName = "deepseek-r1:1.5b";
        c.ollamaModelDesc = "test ollama model desc";
        c.ollamaEndpoint = "127.0.0.1:11434";
        return c;
    }

    Json::Value ParseJson(const std::string& body)
    {
        Json::Value value;
        Json::CharReaderBuilder builder;
        std::string err;
        std::istringstream ss(body);
        if (!Json::parseFromStream(builder, ss, &value, &err))
        {
            return Json::Value();  // 解析失败返回 null
        }
        return value;
    }

    // 通过 HTTP 创建一个会话，返回 session_id；失败返回空串。
    std::string CreateSession(httplib::Client& client, const std::string& model = "deepseek-flash",
                              const std::string& session = "测试会话")
    {
        Json::Value req;
        req["model"] = model;
        req["session"] = session;
        Json::StreamWriterBuilder wb;
        std::string body = Json::writeString(wb, req);

        auto res = client.Post("/api/session", body, "application/json");
        if (!res || res->status != 200)
            return "";

        Json::Value resp = ParseJson(res->body);
        if (resp.isNull() || !resp["success"].asBool())
            return "";
        return resp["data"]["session_id"].asString();
    }
}

// ==================== 生命周期测试 ====================
TEST(ChatServerLifecycleTest, StartStopIsRunning)
{
    auto server = std::make_unique<ChatServer>(MakeConfig(kLifecyclePort));
    ASSERT_NE(server, nullptr);

    // 初始未运行
    EXPECT_FALSE(server->IsRunning());

    // 启动
    EXPECT_TRUE(server->Start());
    EXPECT_TRUE(server->IsRunning());

    // 重复启动应被拒绝
    EXPECT_FALSE(server->Start());
    EXPECT_TRUE(server->IsRunning());

    // 停止
    server->Stop();
    EXPECT_FALSE(server->IsRunning());

    // 重复停止应被拒绝
    server->Stop();
    EXPECT_FALSE(server->IsRunning());
}

// ==================== HTTP 接口测试（共享一个运行中的服务） ====================
class ChatServerEndpointTest : public ::testing::Test
{
protected:
    static std::unique_ptr<ChatServer> s_server;
    static std::unique_ptr<httplib::Client> s_client;

    static void SetUpTestSuite()
    {
        s_server = std::make_unique<ChatServer>(MakeConfig(kServerPort));
        ASSERT_NE(s_server, nullptr);
        ASSERT_TRUE(s_server->Start());

        // 等服务真正开始监听
        std::this_thread::sleep_for(std::chrono::milliseconds(800));

        s_client = std::make_unique<httplib::Client>(kHost, kServerPort);
        s_client->set_connection_timeout(2, 0);
        s_client->set_read_timeout(5, 0);
    }

    static void TearDownTestSuite()
    {
        if (s_server)
        {
            s_server->Stop();
        }
        s_client.reset();
        s_server.reset();
    }
};

std::unique_ptr<ChatServer> ChatServerEndpointTest::s_server;
std::unique_ptr<httplib::Client> ChatServerEndpointTest::s_client;

// POST /api/session 创建会话
TEST_F(ChatServerEndpointTest, CreateSessionSuccess)
{
    Json::Value req;
    req["model"] = "deepseek-flash";
    req["session"] = "单元测试会话";
    Json::StreamWriterBuilder wb;
    std::string body = Json::writeString(wb, req);

    auto res = s_client->Post("/api/session", body, "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    Json::Value resp = ParseJson(res->body);
    ASSERT_FALSE(resp.isNull());
    EXPECT_TRUE(resp["success"].asBool());
    EXPECT_EQ(resp["data"]["model"].asString(), "deepseek-flash");
    EXPECT_EQ(resp["data"]["session"].asString(), "单元测试会话");
    EXPECT_FALSE(resp["data"]["session_id"].asString().empty());
}

// POST /api/session 请求体非法 JSON
TEST_F(ChatServerEndpointTest, CreateSessionInvalidJson)
{
    auto res = s_client->Post("/api/session", "not-a-json", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    Json::Value resp = ParseJson(res->body);
    ASSERT_FALSE(resp.isNull());
    EXPECT_FALSE(resp["success"].asBool());
}

// GET /api/sessions 获取会话列表
TEST_F(ChatServerEndpointTest, GetSessionLists)
{
    std::string sid = CreateSession(*s_client);
    ASSERT_FALSE(sid.empty());

    auto res = s_client->Get("/api/sessions");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    Json::Value resp = ParseJson(res->body);
    ASSERT_FALSE(resp.isNull());
    EXPECT_TRUE(resp["success"].asBool());
    ASSERT_TRUE(resp["data"].isArray());

    bool found = false;
    for (const auto& item : resp["data"])
    {
        if (item["id"].asString() == sid)
        {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// GET /api/models 获取可用模型列表
TEST_F(ChatServerEndpointTest, GetModelLists)
{
    auto res = s_client->Get("/api/models");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    Json::Value resp = ParseJson(res->body);
    ASSERT_FALSE(resp.isNull());
    EXPECT_TRUE(resp["success"].asBool());
    ASSERT_TRUE(resp["data"].isArray());
    EXPECT_FALSE(resp["data"].empty());
}

// GET /api/session/{id}/history 获取会话历史消息
TEST_F(ChatServerEndpointTest, GetHistoryMessages)
{
    std::string sid = CreateSession(*s_client);
    ASSERT_FALSE(sid.empty());

    // 新会话历史应为空数组
    auto res = s_client->Get("/api/session/" + sid + "/history");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    Json::Value resp = ParseJson(res->body);
    ASSERT_FALSE(resp.isNull());
    EXPECT_TRUE(resp["success"].asBool());
    EXPECT_TRUE(resp["data"].isArray());
}

// GET /api/session/{id}/history 会话不存在
TEST_F(ChatServerEndpointTest, GetHistoryMessagesNotFound)
{
    auto res = s_client->Get("/api/session/not-exist-session/history");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);

    Json::Value resp = ParseJson(res->body);
    ASSERT_FALSE(resp.isNull());
    EXPECT_FALSE(resp["success"].asBool());
}

// DELETE /api/session/{id} 删除会话（存在与不存在）
TEST_F(ChatServerEndpointTest, DeleteSession)
{
    std::string sid = CreateSession(*s_client);
    ASSERT_FALSE(sid.empty());

    // 删除存在的会话
    auto res = s_client->Delete("/api/session/" + sid);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    Json::Value resp = ParseJson(res->body);
    ASSERT_FALSE(resp.isNull());
    EXPECT_TRUE(resp["success"].asBool());

    // 删除不存在的会话
    auto res2 = s_client->Delete("/api/session/not-exist-session");
    ASSERT_NE(res2, nullptr);
    EXPECT_EQ(res2->status, 404);

    Json::Value resp2 = ParseJson(res2->body);
    ASSERT_FALSE(resp2.isNull());
    EXPECT_FALSE(resp2["success"].asBool());
}

// POST /api/message 全量发送消息（非法请求体 / 空参数 / 会话不存在）
TEST_F(ChatServerEndpointTest, SendMessageInvalidRequest)
{
    // 非法 JSON
    {
        auto res = s_client->Post("/api/message", "not-a-json", "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 400);
        Json::Value resp = ParseJson(res->body);
        ASSERT_FALSE(resp.isNull());
        EXPECT_FALSE(resp["success"].asBool());
    }

    // session_id 或 message 为空
    {
        Json::Value req;
        req["session_id"] = "";
        req["message"] = "";
        Json::StreamWriterBuilder wb;
        std::string body = Json::writeString(wb, req);
        auto res = s_client->Post("/api/message", body, "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 400);
        Json::Value resp = ParseJson(res->body);
        ASSERT_FALSE(resp.isNull());
        EXPECT_FALSE(resp["success"].asBool());
    }

    // 会话不存在 -> sendMessage 返回空 -> 500
    {
        Json::Value req;
        req["session_id"] = "not-exist-session";
        req["message"] = "你好";
        Json::StreamWriterBuilder wb;
        std::string body = Json::writeString(wb, req);
        auto res = s_client->Post("/api/message", body, "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 500);
        Json::Value resp = ParseJson(res->body);
        ASSERT_FALSE(resp.isNull());
        EXPECT_FALSE(resp["success"].asBool());
    }
}

// POST /api/message/async 流式发送消息（非法请求体 / 空参数）
TEST_F(ChatServerEndpointTest, SendMessageStreamInvalidRequest)
{
    // 非法 JSON
    {
        auto res = s_client->Post("/api/message/async", "not-a-json", "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 400);
        Json::Value resp = ParseJson(res->body);
        ASSERT_FALSE(resp.isNull());
        EXPECT_FALSE(resp["success"].asBool());
    }

    // session_id 或 message 为空
    {
        Json::Value req;
        req["session_id"] = "";
        req["message"] = "";
        Json::StreamWriterBuilder wb;
        std::string body = Json::writeString(wb, req);
        auto res = s_client->Post("/api/message/async", body, "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 400);
        Json::Value resp = ParseJson(res->body);
        ASSERT_FALSE(resp.isNull());
        EXPECT_FALSE(resp["success"].asBool());
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    LogModule::SpdLogPack::SpdLogInit("ChatServerTestLog",
                                      "ChatServerTestLog.dat",
                                      spdlog::level::info,
                                      CONSOLE__MODE);

    return RUN_ALL_TESTS();
}