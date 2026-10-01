#include "ChatServer.h"
#include <jsoncpp/json/forwards.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>
namespace  ChatServerModule
{
    ChatServer::ChatServer(const ServerConfig configs)
    {
        _chatSDK = std::make_shared<Cplusplus_LLM_Provider::ChatSDK>();

        auto deepseekConfig = std::make_shared<Cplusplus_LLM_Provider::APIConfig>();
        deepseekConfig->_modelName = "deepseek-flash";
        deepseekConfig->_apiKey = configs.deepseekAPIKey;
        deepseekConfig->_temperature = configs.temperature;
        deepseekConfig->_maxTokens = configs.maxTokens;


        auto chatGPTConfig = std::make_shared<Cplusplus_LLM_Provider::APIConfig>();
        chatGPTConfig->_modelName = "gpt-5.5";
        chatGPTConfig->_apiKey = configs.chatGPTAPIKey;
        chatGPTConfig->_temperature = configs.temperature;
        chatGPTConfig->_maxTokens = configs.maxTokens;

        auto kimiConfig = std::make_shared<Cplusplus_LLM_Provider::APIConfig>();
        kimiConfig->_modelName = "kimi-k2.6";
        kimiConfig->_apiKey = configs.geminiAPIKey;
        kimiConfig->_temperature = configs.temperature;
        kimiConfig->_maxTokens = configs.maxTokens;

        auto ollamaConfig = std::make_shared<Cplusplus_LLM_Provider::OllamaConfig>();
        ollamaConfig->_modelName = configs.ollamaModelName;
        ollamaConfig->_modelDesc = configs.ollamaModelDesc;
        ollamaConfig->_endpoint = configs.ollamaEndpoint;
        ollamaConfig->_temperature = configs.temperature;
        ollamaConfig->_maxTokens = configs.maxTokens;

        std::vector<std::shared_ptr<Cplusplus_LLM_Provider::Config>> ModelConfigs = {
            deepseekConfig, chatGPTConfig, kimiConfig, ollamaConfig
        };
        LogModule::INFO("start init ChatSDK models...");
        if(!_chatSDK->initModels(ModelConfigs)){
            LogModule::ERROR("ChatSDK init Failed!!!");
            return;
        }
        LogModule::INFO("ChatSDK models init success!!!");

        _chatServer = std::make_unique<httplib::Server>();
        if(!_chatServer){
            LogModule::ERROR("ChatServer init Failed!!!");
            return;
        }
    }
    bool ChatServer::Start()
    {
        if(_isRunning.load()){
            LogModule::ERROR("ChatServer is running!!!");
            return false;
        }

        setHttpRoutes();
        _chatServer->set_mount_point("/", "./www");

        std::thread serverThread([this](){
            _chatServer->listen(_configs.host, _configs.port);
            LogModule::INFO("ChatServer start on {} :{}", _configs.host, _configs.port);
        });

        serverThread.detach();
        _isRunning.store(true);
        LogModule::INFO("ChatServer start success!");
        return true;
    }
    void ChatServer::Stop()
    {
        if(!_isRunning.load()){
            LogModule::ERROR("ChatServer is not running!!!");
            return;
        }

        if(_chatServer){
            _chatServer->stop();
        }

        _isRunning.store(false);
        LogModule::INFO("ChatServer stop success!!!");
    }
    bool  ChatServer::IsRunning()
    {
        return _isRunning ;
    }

    std::string ChatServer::buildResponse(const std::string& message, bool success = false)
    {
        Json::Value responseJson;
        responseJson["success"] = success;
        responseJson["message"] = message;

        // 序列化
        Json::StreamWriterBuilder writerBuilder;
        return Json::writeString(writerBuilder, responseJson);
    }
    // 处理创建会话请求
    void ChatServer::handleCreateSessionRequest(const httplib::Request& request, httplib::Response& response)
    {
        Json::Value requestJson;
        Json::Reader reader;
        if(!reader.parse(request.body, requestJson)){
            std::string errorJsonStr = buildResponse("parse request body failed, json format error");
            response.status = 400; // 客户端发送的请求有语法错误，服务器无法理解或处理该请求
            response.set_content(errorJsonStr, "application/json");
            return;
        }

        std::string modelName = requestJson.get("model", "deepseek-flash").asString();
        std::string sessionName =  requestJson.get("session", "默认会话").asString();

        std::string sessionID = _chatSDK->createSession(sessionName, modelName);
        if(sessionID.empty()){
            std::string errorJsonStr = buildResponse("create session failed");
            response.status = 500; // 服务器内部错误，无法完成请求
            response.set_content(errorJsonStr, "application/json");
            return;
        }
        // 构建响应体
        Json::Value dataJson;
        dataJson["session_id"] = sessionID;
        dataJson["model"] = modelName;
        dataJson["session"] = sessionName;
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "create session success";
        responseJson["data"] = dataJson;

        // 序列化
        Json::StreamWriterBuilder writerBuilder;
        std::string responseJsonStr = Json::writeString(writerBuilder, responseJson);

        response.status = 200; // 成功
        response.set_content(responseJsonStr, "application/json");
    }
    // 处理获取会话列表请求
    void ChatServer::handleGetSessionListsRequest(const httplib::Request& request, httplib::Response& response)
    {
        // 获取会话列表
        std::vector<std::string> sessionIDs = _chatSDK->getSessionLists();

        // 构建session信息
        Json::Value dataArray(Json::arrayValue);
        for(const auto& sessionID : sessionIDs){
            auto session = _chatSDK->getSession(sessionID);
            if(session){
                Json::Value sessionJson;
                sessionJson["id"] = session->_SessionID;
                sessionJson["model"] = session->_ModelNameUsed;
                sessionJson["created_at"] = static_cast<int64_t>(session->_TimeCreate);
                sessionJson["updated_at"] = static_cast<int64_t>(session->_LastTime);
                sessionJson["message_count"] = session->_Messages.size();
                if(!session->_Messages.empty()){
                    sessionJson["first_user_message"] = session->_Messages.front()._Content;
                }
                dataArray.append(sessionJson);
            }
        }

        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get session lists success";
        responseJson["data"] = dataArray;

        Json::StreamWriterBuilder writerBuilder;
        std::string responseJsonStr = Json::writeString(writerBuilder, responseJson);

        response.status = 200; 
        response.set_content(responseJsonStr, "application/json");
    }
    // 处理获取模型列表请求
    void ChatServer::handleGetModelListsRequest(const httplib::Request& request, httplib::Response& response)
    {
        auto modelLists = _chatSDK->getAvailableModels();
        Json::Value dataArray(Json::arrayValue);
        for(const auto& modelInfo : modelLists){
            Json::Value modelJson;
            modelJson["name"] = modelInfo.first;
            modelJson["desc"] = modelInfo.second._ModelDesc;
            dataArray.append(modelJson);
        }
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get model lists success";
        responseJson["data"] = dataArray;

        Json::StreamWriterBuilder writerBuilder;
        std::string responseJsonStr = Json::writeString(writerBuilder, responseJson);

        response.status = 200; 
        response.set_content(responseJsonStr, "application/json");
    }
    // 处理删除会话请求
    void ChatServer::handleDeleteSessionRequest(const httplib::Request& request, httplib::Response& response)
    {
        std::string sessionId = request.matches[1];
        
        bool ret = _chatSDK->deleteSession(sessionId);
        if(ret){
            std::string errorJsonStr = buildResponse("delete session success", true);
            response.status = 200; 
            response.set_content(errorJsonStr, "application/json");
        }else{
            std::string errorJsonStr = buildResponse("delete session failed, session not found");
            response.status = 404;  // 会话不存在
            response.set_content(errorJsonStr, "application/json");
        }
    }
    // 处理获取历史消息请求
    void ChatServer::handleGetHistoryMessagesRequest(const httplib::Request& request, httplib::Response& response)
    {
        // 获取会话id
        std::string sessionId = request.matches[1];
        // 获取会话
        auto session = _chatSDK->getSession(sessionId);
        if(!session){
            std::string errorJsonStr = buildResponse("session not found");
            response.status = 404;  
            response.set_content(errorJsonStr, "application/json");
            return;
        }

        // 构建历史消息列表
        Json::Value dataArray(Json::arrayValue);
        for(const auto& message : session->_Messages){
            Json::Value messageJson;
            messageJson["id"] = message._MessageID;
            messageJson["role"] = message._Role;
            messageJson["content"] = message._Content;
            messageJson["timestamp"] = static_cast<int64_t>(message._Time);
            dataArray.append(messageJson);
        }

        // 构建响应体
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "get history messages success";
        responseJson["data"] = dataArray;

        // 序列化
        Json::StreamWriterBuilder writerBuilder;
        std::string responseJsonStr = Json::writeString(writerBuilder, responseJson);

        response.status = 200; // 成功
        response.set_content(responseJsonStr, "application/json");
    }
    // 处理发送消息请求-全量返回
    void ChatServer::handleSendMessageRequest(const httplib::Request& request, httplib::Response& response)
    {
        // 获取请求参数
        Json::Value requestJson;
        Json::Reader reader;
        if(!reader.parse(request.body, requestJson)){
            std::string errorJsonStr = buildResponse("parse request body failed, json format error");
            response.status = 400;  // 解析请求参数失败
            response.set_content(errorJsonStr, "application/json");
            return;
        }

        // 解析请求参数
        std::string sessionId = requestJson["session_id"].asString();
        std::string message = requestJson["message"].asString();
        if(sessionId.empty() || message.empty()){
            std::string errorJsonStr = buildResponse("session_id or message is empty");
            response.status = 400;  // 解析请求参数失败
            response.set_content(errorJsonStr, "application/json");
            return;
        }

        // 发送消息
        std::string assistantMessage = _chatSDK->sendMessage(sessionId, message);
        if(assistantMessage.empty()){
            std::string errorJsonStr = buildResponse("Failed to send AI response message");
            response.status = 500;  // 发送消息失败
            response.set_content(errorJsonStr, "application/json");
            return;
        }

        // 构造响应参数
        Json::Value dataJson;
        dataJson["session_id"] = sessionId;
        dataJson["response"] = assistantMessage;
        dataJson["data"]["assistant_message"] = assistantMessage;

        // 构建响应体
        Json::Value responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "send message success";
        responseJson["data"] = dataJson;

        // 序列化
        Json::StreamWriterBuilder writerBuilder;
        std::string responseJsonStr = Json::writeString(writerBuilder, responseJson);

        response.status = 200; // 成功
        response.set_content(responseJsonStr, "application/json");
    }
    // 处理发送消息请求-增量返回
    void ChatServer::handleSendMessageStreamRequest(const httplib::Request& request, httplib::Response& response)
    {
        Json::Value requestJson;
        Json::Reader reader;
        if(!reader.parse(request.body, requestJson)){
            std::string errorJsonStr = buildResponse("parse request body failed, json format error");
            response.status = 400; 
            response.set_content(errorJsonStr, "application/json");
            return;
        }

        std::string sessionId = requestJson["session_id"].asString();
        std::string message = requestJson["message"].asString();
        if(sessionId.empty() || message.empty()){
            std::string errorJsonStr = buildResponse("session_id or message is empty");
            response.status = 400; 
            response.set_content(errorJsonStr, "application/json");
            return;
        }

        // 准备流式响应
        response.status = 200;
        response.set_header("Cache-Control", "no-cache");              // 不使用缓存，服务器立即将数据发送到网络
        response.set_header("Connection", "keep-alive");               // 保持连接，服务器不会关闭连接
        response.set_header("Access-Control-Allow-Origin", "*");        // 允许跨域请求
        response.set_header("Access-Control-Allow-Headers", "*");      // 允许所有请求头
        
        // set_chunked_content_provider：告诉服务器，响应内从不是一次性发送的,
        //而是分多次逐步发送给客户端，一般用在实时生成响应内容 或者 流式数据传输场景
        response.set_chunked_content_provider("text/event-stream", [this, sessionId, message](size_t offset, httplib::DataSink& dataSink)->bool{

            auto writeChunk = [&](const std::string& chunk, bool last){ 
                // 将chunk转换为SSE数据格式
                // Json::valueToQuotedString: 对chunk进行Json转换，
                //目的防止chunk中包含一些特殊字符来破坏数据格式，比如：在chunk中包含了两个连续的换行，就会影响SSE数据格式
                std::string sseData = "data: " + Json::valueToQuotedString(chunk.c_str()) + "\n\n";

                // 需要将模型返回的结果 chunk 发送给客户单
                dataSink.write(sseData.c_str(), sseData.size());  // 将数据写入响应流，即立即发送给客户单，该方法不会等待缓冲区满之后发送

                // 处理结束标记
                if(last){
                    // 流向响应结束
                    std::string doneData = "data: [DONE]\n\n";
                    dataSink.write(doneData.c_str(), doneData.size());
                    dataSink.done();    // 表示流式响应结束
                    return false;       // 不再有后续数据
                }
                return true;
            };
            
            // 先给客户端发送一个空的数据块，避免客户端长时间的等待
            if (!writeChunk("", false)) {
                return false;
            }
            
            // 发送消息流
            _chatSDK->sendMessageStream(sessionId, message, writeChunk);
            return false;   // 不再有后续数据
        });
    }

    // 设置HTTP路由规则
    void ChatServer::setHttpRoutes()
    {
        // 处理创建会话请求
        _chatServer->Post("/api/session", [this](const httplib::Request& request, httplib::Response& response){
            handleCreateSessionRequest(request, response);
        });

        // 处理获取会话列表请求
        _chatServer->Get("/api/sessions", [this](const httplib::Request& request, httplib::Response& response){
            handleGetSessionListsRequest(request, response);
        }); 

        // 处理获取模型列表请求
        _chatServer->Get("/api/models", [this](const httplib::Request& request, httplib::Response& response){
            handleGetModelListsRequest(request, response);
        });

        // 处理删除会话请求
        _chatServer->Delete("/api/session/(.*)", [this](const httplib::Request& request, httplib::Response& response){
            handleDeleteSessionRequest(request, response);
        });

        // 处理获取历史消息请求
        _chatServer->Get("/api/session/(.*)/history", [this](const httplib::Request& request, httplib::Response& response){
            handleGetHistoryMessagesRequest(request, response);
        });

        // 处理发送消息请求-全量返回
        _chatServer->Post("/api/message", [this](const httplib::Request& request, httplib::Response& response){
            handleSendMessageRequest(request, response);
        });

        // 处理发送消息请求-增量返回
        _chatServer->Post("/api/message/async", [this](const httplib::Request& request, httplib::Response& response){
            handleSendMessageStreamRequest(request, response);
        });
    }
}