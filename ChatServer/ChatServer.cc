#include "ChatServer.h"
#include <thread>
#include <fstream>
#include <random>
namespace ChatServerModule
{
    // 前向声明：供 RouterRegister 中的静态资源路由提前调用
    void SendStaticResource(httplib::Response& response,
        const std::string& relativePath,const std::string& contentType);

    bool ChatServer::InitChatServer(SdkConfig& skdconfigs)
    {
        if(_isrunning == true){
            LogModule::ERROR("ChatServer已初始化,请不要重复初始化!");
            return false ;
        }
        LogModule::SpdLogPack::SpdLogInit(
            "ChatServerLog",
            "ServerLog.dat",
            spdlog::level::info,
            ROTATING_MODE
        );
        //服务器初始化
        if(_chatserver != nullptr){
            LogModule::ERROR("服务器已初始化,请不要重复初始化!");
            return false ;
        }
        _chatserver = std::make_shared<httplib::Server>();
        _chatsdk = std::make_shared<ChatSDK>();
        //配置参数
        std::vector<std::shared_ptr<Config>> Modelskdconfigs;

        std::shared_ptr<APIConfig> deepseekConfig = std::make_shared<APIConfig>();
        deepseekConfig->_apiKey = skdconfigs._DeepseekApiKey;
        deepseekConfig->_maxTokens = skdconfigs._MaxTokens;
        deepseekConfig->_modelName = skdconfigs._DeepseekModelName;
        deepseekConfig->_temperature = skdconfigs._Temperature;
        Modelskdconfigs.push_back(deepseekConfig);

        std::shared_ptr<APIConfig> ChaGptConfig = std::make_shared<APIConfig>();
        ChaGptConfig->_apiKey = skdconfigs._ChatGPTApiKey;
        ChaGptConfig->_maxTokens = skdconfigs._MaxTokens;
        ChaGptConfig->_modelName = skdconfigs._ChatGPTModelName;
        ChaGptConfig->_temperature = skdconfigs._Temperature;
        Modelskdconfigs.push_back(ChaGptConfig);

        std::shared_ptr<APIConfig> KimiConfig = std::make_shared<APIConfig>();
        KimiConfig->_apiKey = skdconfigs._KimiApiKey;
        KimiConfig->_maxTokens = skdconfigs._MaxTokens;
        KimiConfig->_modelName = skdconfigs._KimiModelName;
        KimiConfig->_temperature = skdconfigs._Temperature;
        Modelskdconfigs.push_back(KimiConfig);

        std::shared_ptr<OllamaConfig> ollamaConfig = std::make_shared<OllamaConfig>();
        ollamaConfig->_maxTokens = skdconfigs._MaxTokens;
        ollamaConfig->_temperature = skdconfigs._Temperature;
        ollamaConfig->_modelName = skdconfigs._OllamaModelName;
        ollamaConfig->_endpoint = skdconfigs._OllamaEndpoint;
        ollamaConfig->Num_Ctx = skdconfigs._OllamaNumCtx;
        ollamaConfig->_modelDesc = skdconfigs._OllamaModelDesc;
        Modelskdconfigs.push_back(ollamaConfig);
        if(_chatsdk->initModels(Modelskdconfigs))
            LogModule::INFO("ChatSDK初始化成功!");
        
        _isrunning = true ;
        return true ;
    }
    bool ChatServer::Start(ChatServerConfig& serverConfigs)
    {
        if(_isrunning == false){
            LogModule::ERROR("服务器未初始化!");
            return false ;
        }
        RouterRegister();//设置路由规则
        std::thread ServerThread([this, &serverConfigs]{
            if(!_chatserver->listen(serverConfigs._host,serverConfigs._port)){
                LogModule::ERROR("服务器监听出现异常!");
            }
        }) ;
        ServerThread.detach();
        return true;
    }
    bool ChatServer::Stop()
    {
        if(_isrunning == true){
            _isrunning = false;
            LogModule::INFO("服务器关闭成功!");
            return true ;
        }
        return false ;
    }
    //注册所有的路由方案
    void ChatServer::RouterRegister()
    {
        //前后端约定一套URL规则:
        // GET /pages/home.html
        // GET /pages/register.html
        // GET /pages/chatroom.html
        // GET /images/*
        
        // POST /api/sessions 
        // GET /api/sessionlists
        // GET /api/session/${session_id}/history
        // GET /api/models
        // POST /api/message
        // POST /api/stream
        // POST /api/delsession
        // POST /api/register

        //基础页面
        _chatserver->Get("/pages/home.html",[this](const httplib::Request& request, httplib::Response& response){
            HandleGetHomePage(request,response);
        });
        _chatserver->Get("/pages/register.html",[this](const httplib::Request& request, httplib::Response& response){
            HandleGetRegisterPage(request,response);
        });
        _chatserver->Get("/pages/chatroom.html",[this](const httplib::Request& request, httplib::Response& response){
            HandleGetChatRoomPage(request,response);
        });
        _chatserver->Get(R"(/images/.*)",[this](const httplib::Request& request, httplib::Response& response){
            HandleImage(request,response);
        });
        _chatserver->Get(R"(/css/.*)",[this](const httplib::Request& request, httplib::Response& response){
            SendStaticResource(response,request.path,"text/css; charset=utf-8");
        });
        _chatserver->Get(R"(/js/.*)",[this](const httplib::Request& request, httplib::Response& response){
            SendStaticResource(response,request.path,"application/javascript; charset=utf-8");
        });
        _chatserver->Post("/api/sessions",[this](const httplib::Request& request, httplib::Response& response){
            HandleCreateSession(request,response);
        });
        _chatserver->Get("/api/sessionlists",[this](const httplib::Request& request, httplib::Response& response){
            HandleGetSessionLists(request,response);
        });
        _chatserver->Get(R"(/api/session/.*/history)",[this](const httplib::Request& request, httplib::Response& response){
            HandleGetHistoryMessages(request,response);
        });
        _chatserver->Get("/api/models",[this](const httplib::Request& request, httplib::Response& response){
            HandleGetAvailableModel(request,response);
        });
        _chatserver->Post("/api/message",[this](const httplib::Request& request, httplib::Response& response){
            HandleSendMessages(request,response);
        });
        _chatserver->Post("/api/stream",[this](const httplib::Request& request, httplib::Response& response){
            HandleSendMessagesAsStream(request,response);
        });
        _chatserver->Post("/api/delsession",[this](const httplib::Request& request, httplib::Response& response){
            HandleDeleteSession(request,response);
        });
        _chatserver->Post("/api/register",[this](const httplib::Request& request, httplib::Response& response){
            HandleRegister(request,response);
        });
    }


    void ChatServer::SendJsonResponse(httplib::Response& response,int status,bool success,
        const std::string& message,const Json::Value& data)
    {
        Json::Value responseJson;
        responseJson["success"] = success;
        responseJson["message"] = message;
        responseJson["data"] = data;
        Json::StreamWriterBuilder writer;

        response.status = status;
        response.set_header("Cache-Control", "no-store");
        response.set_content(
            Json::writeString(writer, responseJson),
            "application/json; charset=utf-8"
        );
    }
    //参考 Argon2id 基本思想的简化密码哈希方案(仅用于学习与演示,不符合Argon2id标准,不得用于生产环境)
    std::string SimplePasswordHash(const std::string& password, const std::string& salt)
    {
        std::hash<std::string> hasher;
        std::string data = password + salt;
        for(int i = 0 ; i < 100000 ; ++i)
            data = std::to_string(hasher(data + salt + std::to_string(i)));
        std::stringstream ss;
        ss << std::hex << hasher(data);
        return ss.str();
    }
    //生成随机盐
    std::string GenerateSalt()
    {
        std::random_device rd;
        std::stringstream ss;
        ss << std::hex << rd() << rd();
        return ss.str();
    }
    //根据图片扩展名获取对应的Content-Type
    std::string GetImageContentType(const std::string& path)
    {
        const size_t pos = path.find_last_of('.');
        const std::string ext = (pos == std::string::npos) ? "" : path.substr(pos);
        if(ext == ".png")
            return "image/png";
        if(ext == ".gif")
            return "image/gif";
        if(ext == ".svg")
            return "image/svg+xml";
        if(ext == ".webp")
            return "image/webp";
        if(ext == ".ico")
            return "image/x-icon";
        return "image/jpeg";
    }
    //从www目录读取静态资源并输出
    void SendStaticResource(httplib::Response& response,
        const std::string& relativePath,const std::string& contentType)
    {
        if(relativePath.find("..") != std::string::npos){
            response.status = 404;
            response.set_content("404 Not Found","text/plain; charset=utf-8");
            return;
        }
        std::ifstream file("./www" + relativePath,std::ios::binary);
        if(!file.is_open()){
            response.status = 404;
            response.set_content("404 Not Found","text/plain; charset=utf-8");
            return;
        }
        std::ostringstream oss;
        oss << file.rdbuf();
        response.status = 200;
        response.set_content(oss.str(),contentType);
    }
    void ChatServer::HandleGetHomePage(const httplib::Request&, httplib::Response& response)
    {
        SendStaticResource(response,"/pages/home.html","text/html; charset=utf-8");
    }
    void ChatServer::HandleGetRegisterPage(const httplib::Request&, httplib::Response& response)
    {
        SendStaticResource(response,"/pages/register.html","text/html; charset=utf-8");
    }
    void ChatServer::HandleGetChatRoomPage(const httplib::Request&, httplib::Response& response)
    {
        SendStaticResource(response,"/pages/chatroom.html","text/html; charset=utf-8");
    }
    void ChatServer::HandleImage(const httplib::Request& request, httplib::Response& response)
    {
        SendStaticResource(response,request.path,GetImageContentType(request.path));
    }
    //请求处理

    void ChatServer::HandleCreateSession(
        const httplib::Request& request,
        httplib::Response& response
    )
    {
        //请求
        // {
        //     "model" : "string",
        //     "session" : "string"
        // }
        //应答
        // {
        //     "success" : "bool",
        //     "message" : "string",
        //     "data":
        //     {
        //         "session_id" : "string",
        //         "model" : "string"
        //         "session" : "string"
        //     }
        // }
        Json::Value data;
        data["session_id"] = "";
        data["model"] = "";
        data["session"] = "";
        if (!_isrunning){
            SendJsonResponse(response, 500, false, "ChatServer is not running", data);
            return;
        }
        if (request.body.empty()){
            SendJsonResponse(response, 400, false,"Request Body is empty", data);
            return;
        }
        Json::CharReaderBuilder readerBuilder;
        Json::Value requestJson;
        std::string errors;
        std::istringstream stream(request.body);
        if (!Json::parseFromStream(
                readerBuilder, stream, &requestJson, &errors)){
            SendJsonResponse(
                response, 400, false,
                "Invalid JSON format", data
            );
            return;
        }
        if (!requestJson.isObject() ||
             !requestJson["model"].isString() ||
              !requestJson["session"].isString() ||
                requestJson["model"].asString().empty() ||
                 requestJson["session"].asString().empty()){
            SendJsonResponse(
                response, 400, false,
                "model or session is empty or invalid", data
            );
            return;
        }
        const std::string modelName = requestJson["model"].asString();
        const std::string sessionName = requestJson["session"].asString();
        const std::string userName = requestJson.isMember("username") &&
            requestJson["username"].isString()
            ? requestJson["username"].asString() : "";
        const std::string sessionId =
            _chatsdk->createSession(sessionName, modelName, userName);
        data["session_id"] = sessionId;
        data["model"] = modelName;
        data["session"] = sessionName;
        SendJsonResponse(
            response, 200, true,
            "OK", data
        );
    }
    void ChatServer::HandleGetSessionLists(const httplib::Request& request, httplib::Response& response)
    {
        //应答
        // {
        //     "success" : "bool",
        //     "message" : "string",
        //     "data": "array"[
        //         {
        //         "id" : "string",
        //         "model" : "string",
        //         "created_at" : "int64_t",
        //         "updated_at" : "int64_t",
        //         "message_count" : "int",
        //         "first_user_message" : "string"
        //         }
        //     ]
        // }
        Json::Value data(Json::arrayValue);
        if (!_isrunning){
            SendJsonResponse(response, 500, false, "ChatServer is not running", data);
            return;
        }
        const std::vector<std::string> sessionIds =
            _chatsdk->getSessionLists();
        for (const auto& sessionId : sessionIds){
            std::shared_ptr<Cplusplus_LLM_Provider::Session> sessionPtr =
                _chatsdk->getSession(sessionId);
            if (sessionPtr == nullptr)
                continue;
            Json::Value item;
            item["id"] = sessionPtr->_SessionID;
            item["model"] = sessionPtr->_ModelNameUsed;
            item["created_at"] = static_cast<Json::Int64>(sessionPtr->_TimeCreate);
            item["updated_at"] = static_cast<Json::Int64>(sessionPtr->_LastTime);
            item["message_count"] = static_cast<int>(sessionPtr->_Messages.size());
            std::string firstUserMessage;
            for (const auto& msg : sessionPtr->_Messages){
                if (msg._Role == "user"){
                    firstUserMessage = msg._Content;
                    break;
                }
            }
            item["first_user_message"] = firstUserMessage;
            data.append(item);
        }
        SendJsonResponse(
            response, 200, true,
            "OK", data
        );
    }
    void ChatServer::HandleGetAvailableModel(const httplib::Request& request, httplib::Response& response)
    {
        //应答
        // {
        //     "success" : "bool",
        //     "message" : "string",
        //     "data": "array"[
        //         {
        //         "name" : "string",
        //         "desc" : "string"
        //         }
        //     ]
        // }
        Json::Value data(Json::arrayValue);
        if (!_isrunning){
            SendJsonResponse(response, 500, false, "ChatServer is not running", data);
            return;
        }
        const std::vector<std::pair<std::string,Cplusplus_LLM_Provider::ModelInfo>>
            models = _chatsdk->getAvailableModels();
        for (const auto& model : models){
            if (!model.second._IsThisModelAvailable)
                continue;
            Json::Value item;
            item["name"] = model.first;
            item["desc"] = model.second._ModelDesc;
            data.append(item);
        }
        SendJsonResponse(
            response, 200, true,
            "OK", data
        );
    }
    void ChatServer::HandleDeleteSession(const httplib::Request& request, httplib::Response& response)
    {
        //请求
        // {
        //     "session_id" : "string"
        // }
        //应答
        // {
        //     "success" : "bool",
        //     "message" : "string"
        // }
        Json::Value data;
        if (!_isrunning){
            SendJsonResponse(response, 500, false, "ChatServer is not running", data);
            return;
        }
        if (request.body.empty()){
            SendJsonResponse(response, 400, false,"Request Body is empty", data);
            return;
        }
        Json::CharReaderBuilder readerBuilder;
        Json::Value requestJson;
        std::string errors;
        std::istringstream stream(request.body);
        if (!Json::parseFromStream(
                readerBuilder, stream, &requestJson, &errors)){
            SendJsonResponse(
                response, 400, false,
                "Invalid JSON format", data
            );
            return;
        }
        if (!requestJson.isObject() ||
             !requestJson["session_id"].isString() ||
              requestJson["session_id"].asString().empty()){
            SendJsonResponse(
                response, 400, false,
                "session_id is empty or invalid", data
            );
            return;
        }
        const std::string sessionId = requestJson["session_id"].asString();
        if (!_chatsdk->deleteSession(sessionId)){
            SendJsonResponse(
                response, 404, false,
                "Session not found or delete failed", data
            );
            return;
        }
        SendJsonResponse(
            response, 200, true,
            "OK", data
        );
    }
    void ChatServer::HandleGetHistoryMessages(const httplib::Request& request, httplib::Response& response)
    {
        //应答
        // {
        //     "success" : "bool",
        //     "message" : "string",
        //     "data": "array"[
        //         {
        //         "id" : "string",
        //         "role" : "string",
        //         "content" : "string",
        //         "timestamp" : 0,
        //         }
        //     ]
        // }
        Json::Value data(Json::arrayValue);
        if (!_isrunning){
            SendJsonResponse(response, 500, false, "ChatServer is not running", data);
            return;
        }
        //从路径 /api/session/{session_id}/history 中提取 session_id
        const std::string endpoint = "/api/session/";
        const std::string suffix = "/history";
        const std::string& path = request.path;
        const size_t start = path.find(endpoint);
        const size_t end = path.rfind(suffix);
        if (start == std::string::npos ||
             end == std::string::npos ||
               end <= start + endpoint.size()){
            SendJsonResponse(
                response, 400, false,
                "Invalid session url", data
            );
            return;
        }
        const std::string sessionId = path.substr(
            start + endpoint.size(), end - (start + endpoint.size()));
        std::shared_ptr<Cplusplus_LLM_Provider::Session> sessionPtr =
            _chatsdk->getSession(sessionId);
        if (sessionPtr == nullptr){
            SendJsonResponse(
                response, 404, false,
                "Session not found", data
            );
            return;
        }
        const std::vector<Cplusplus_LLM_Provider::Message> messages =
            sessionPtr->_Messages;
        for (const auto& msg : messages){
            Json::Value item;
            item["id"] = msg._MessageID;
            item["role"] = msg._Role;
            item["content"] = msg._Content;
            item["timestamp"] = static_cast<Json::Int64>(msg._Time);
            data.append(item);
        }
        SendJsonResponse(
            response, 200, true,
            "OK", data
        );
    }
    void ChatServer::HandleSendMessages(const httplib::Request& request, httplib::Response& response)
    {
        //请求
        // {
        //     "session_id" : "string",
        //     "message" : "string"
        // }
        //应答
        // {
        //     "success" : "bool",
        //     "message" : "string",
        //     "data":
        //     {
        //         "session_id" : "string",
        //         "response" : "string"
        //     }
        // }
        Json::Value data;
        data["reply"] = "";
        if (!_isrunning){
            SendJsonResponse(response, 500, false, "ChatServer is not running", data);
            return;
        }
        if (request.body.empty()){
            SendJsonResponse(response, 400, false,"Request Body is empty", data);
            return;
        }
        Json::CharReaderBuilder readerBuilder;
        Json::Value requestJson;
        std::string errors;
        std::istringstream stream(request.body);
        if (!Json::parseFromStream(
                readerBuilder, stream, &requestJson, &errors)){
            SendJsonResponse(
                response, 400, false,
                "Invalid JSON format", data
            );
            return;
        }
        if (!requestJson.isObject() ||
             !requestJson["session_id"].isString() ||
              !requestJson["message"].isString() ||
                requestJson["session_id"].asString().empty() ||
                 requestJson["message"].asString().empty()){
            SendJsonResponse(
                response, 400, false,
                "session_id or message is empty or invalid", data
            );
            return;
        }
        const std::string sessionId = requestJson["session_id"].asString();
        const std::string message = requestJson["message"].asString();
        const std::string reply =
            _chatsdk->sendMessage(sessionId, message);
        if (reply.empty()){
            SendJsonResponse(
                response, 500, false,
                "Send message failed or session not exists", data
            );
            return;
        }
        data["reply"] = reply;
        SendJsonResponse(
            response, 200, true,
            "OK", data
        );
    }
    void ChatServer::HandleSendMessagesAsStream(const httplib::Request& request, httplib::Response& response)
    {
        //请求
        // {
        //     "session_id" : "string",
        //     "message" : "string"
        // }
        //应答
        // text/event-stream（SSE 流式返回）
        if (!_isrunning){
            Json::Value data;
            SendJsonResponse(response, 500, false, "ChatServer is not running", data);
            return;
        }
        if (request.body.empty()){
            Json::Value data;
            SendJsonResponse(response, 400, false, "Request Body is empty", data);
            return;
        }
        Json::CharReaderBuilder readerBuilder;
        Json::Value requestJson;
        std::string errors;
        std::istringstream stream(request.body);
        if (!Json::parseFromStream(
                readerBuilder, stream, &requestJson, &errors)){
            Json::Value data;
            SendJsonResponse(
                response, 400, false,
                "Invalid JSON format", data
            );
            return;
        }
        if (!requestJson.isObject() ||
             !requestJson["session_id"].isString() ||
              !requestJson["message"].isString() ||
                requestJson["session_id"].asString().empty() ||
                 requestJson["message"].asString().empty()){
            Json::Value data;
            SendJsonResponse(
                response, 400, false,
                "session_id or message is empty or invalid", data
            );
            return;
        }
        const std::string sessionId = requestJson["session_id"].asString();
        const std::string message = requestJson["message"].asString();

        //准备流式响应
        response.status = 200;
        response.set_header("Cache-Control", "no-store");
        response.set_header("Connection", "keep-alive");
        response.set_header("Access-Control-Allow-Origin", "*");
        response.set_header("Access-Control-Allow-Headers", "*");

        // set_chunked_content_provider: 分批推送响应内容，适用于流式/实时生成场景
        response.set_chunked_content_provider(
            "text/event-stream",
            [this, sessionId, message](size_t offset, httplib::DataSink& dataSink)->bool{
                auto writeChunk = [&dataSink](const std::string& chunk, bool last)->bool{
                    // 转成SSE格式，valueToQuotedString 防止特殊字符破坏SSE格式
                    std::string sseData =
                        "data: " + Json::valueToQuotedString(chunk.c_str()) + "\n\n";
                    dataSink.write(sseData.c_str(), sseData.size());
                    if (last){
                        std::string doneData = "data: [DONE]\n\n";
                        dataSink.write(doneData.c_str(), doneData.size());
                        dataSink.done();
                        return false;
                    }
                    return true;
                };
                // 先推送一个空数据块，避免客户端长时间等待
                if (!writeChunk("", false))
                    return false;
                // 发送消息流
                _chatsdk->sendMessageStream(sessionId, message, writeChunk);
                return false;
            }
        );
    }
    void ChatServer::HandleRegister(const httplib::Request& request, httplib::Response& response)
    {
        //请求
        // {
        //     "username" : "string",
        //     "password" : "string"
        // }
        //应答
        // {
        //     "success" : "bool",
        //     "message" : "string",
        //     "data":
        //     {
        //         "username" : "string",
        //         "action" : "register" | "login"
        //     }
        // }
        Json::Value data;
        data["username"] = "";
        data["action"] = "";
        data["sessions"] = Json::arrayValue;
        if (!_isrunning){
            SendJsonResponse(response, 500, false, "ChatServer is not running", data);
            return;
        }
        if (request.body.empty()){
            SendJsonResponse(response, 400, false, "Request Body is empty", data);
            return;
        }
        Json::CharReaderBuilder readerBuilder;
        Json::Value requestJson;
        std::string errors;
        std::istringstream stream(request.body);
        if (!Json::parseFromStream(
                readerBuilder, stream, &requestJson, &errors)){
            SendJsonResponse(response, 400, false, "Invalid JSON format", data);
            return;
        }
        if (!requestJson.isObject() ||
             !requestJson["username"].isString() ||
              !requestJson["password"].isString() ||
                requestJson["username"].asString().empty() ||
                 requestJson["password"].asString().empty()){
            SendJsonResponse(response, 400, false, "username or password is empty or invalid", data);
            return;
        }
        const std::string username = requestJson["username"].asString();
        const std::string password = requestJson["password"].asString();
        data["username"] = username;

        std::shared_ptr<User> user = _chatsdk->_userManager.FindUser(username);
        if (user == nullptr){
            //用户不存在 -> 注册
            const std::string salt = GenerateSalt();
            const std::string passwordHash = SimplePasswordHash(password, salt);
            if (!_chatsdk->_userManager.AddUser(username, passwordHash, salt)){
                SendJsonResponse(response, 500, false, "Register failed", data);
                return;
            }
            data["action"] = "register";
            SendJsonResponse(response, 200, true, "Register success", data);
            return;
        }
        //用户已存在 -> 验证密码
        const std::string passwordHash = SimplePasswordHash(password, user->_Salt);
        if (passwordHash == user->_PasswordHash){
            data["action"] = "login";
            //恢复该用户的历史会话到会话管理
            auto sessions = _chatsdk->_sessionManager.GetSessionsByUser(username);
            for (const auto& s : sessions)
                data["sessions"].append(s->_SessionID);
            SendJsonResponse(response, 200, true, "Login success", data);
            return;
        }
        data["action"] = "";
        SendJsonResponse(response, 401, false, "Login failed: wrong password", data);
    }
}