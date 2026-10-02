#include "../Include/Common.h"
#include "../Include/Util/LogModule.h"
#include "../Include/OllamaProvider.h"
#include "../Include/LLMProvider.h"

namespace Cplusplus_LLM_Provider
{
    bool OllamaProvider::IsModelAvailable()   
    {
        return GetAvailable() ;
    }
    void OllamaProvider::InitModel(std::shared_ptr<Config> Config)   
    {
        auto ollamaConfig = std::dynamic_pointer_cast<OllamaConfig>(Config);
        if(!ollamaConfig || ollamaConfig->_modelName.empty() || ollamaConfig->_endpoint.empty())
        {
            LogModule::CRITICAL("InitModel Fail!{}","Config 类型错误或 Ollama 配置缺失");
            exit(Exception::INIT_EER);
        }
        _ModelName = ollamaConfig->_modelName;
        _ModelDesc = ollamaConfig->_modelDesc;
        SetAPIAccessAddress(ollamaConfig->_endpoint);
        SetAvailable(true);
    } 
    std::string OllamaProvider::GetModelName() { return _ModelName ;}
    ModelInfo OllamaProvider::GetModelDescription() 
    { 
        ModelInfo info(
            _ModelName,
            _ModelDesc,
            GetAPIAccessAddress()
        );
        info._IsThisModelAvailable = GetAvailable();
        return info ;
    }
    std::string OllamaProvider::Serialize(
        std::vector< Message>& messages,
        std::unordered_map<std::string,std::string>& RequestPrograms,
        bool isstream
    )
    {
        std::string model = "deepseek-r1:1.5b" ;
        double temperature =  1;
        bool stream = isstream;
        int Max_token = 0 ;
        int Num_Ctx = 0 ;
        if(RequestPrograms.find("temperature") != RequestPrograms.end())
            temperature = std::stod(RequestPrograms["temperature"]) ; 
        if(RequestPrograms.find("Max_token") != RequestPrograms.end())
            Max_token = std::stoi(RequestPrograms["Max_token"]) ; 
        if(RequestPrograms.find("Num_Ctx") != RequestPrograms.end())
            Num_Ctx = std::stoi(RequestPrograms["Num_Ctx"]) ; 
        Json::Value BodyValues ;
        for(auto message : messages)
        {
            Json::Value value ;
            value["role"] = message._Role;
            value["content"] = message._Content;
            BodyValues.append(value);
        }

        Json::Value Option ;
        Option["num_ctx"] = Num_Ctx;
        Option["num_predict"] = Max_token;
        Option["temperature"] = temperature;
        Json::Value RequestBody ;
        RequestBody["model"] = model ;
        RequestBody["messages"] = BodyValues ;
        RequestBody["options"] = Option ;
        RequestBody["stream"] = stream ;

        Json::StreamWriterBuilder builder ;
        std::unique_ptr<Json::StreamWriter> writer 
                (builder.newStreamWriter());
        std::ostringstream ss ;
        int CheckWrite = writer->write(RequestBody,&ss);
        if(CheckWrite != 0)
        {
            LogModule::ERROR("Json Writer fail!");
            std::string Excepts("Json Writer fail");
            throw Excepts;
        }
        return ss.str() ; 
    }
    httplib::Client OllamaProvider::CreateClient(int commect_timeout , int read_timeout)
    {
        httplib::Client client(GetAPIAccessAddress());
        client.set_connection_timeout(commect_timeout,0);
        client.set_read_timeout(read_timeout,0);
        return client ;
    }
    httplib::Result OllamaProvider::SendRequestMessage(std::string& RequestBodyString)
    {
        httplib::Client client = CreateClient(30 , 60); 
        httplib::Headers handers = {
            {"Content-Type" , "application/json"},
            //{"Authorization" , "Bearer " + GetApiKey() }
        };
        httplib::Result answer = client.Post( "/api/chat",
             handers, RequestBodyString,
             "application/json");  
        if(answer == nullptr)
        {
            std::cerr << "HTTP Request Error: "
                    << httplib::to_string(answer.error())
                    << std::endl;
            LogModule::ERROR("Post Request Failed!");
            return answer;
        }
        std::cout<<"[ answer->body ]: "<<answer->body<<std::endl;
        if(answer != nullptr)
        {
            if(answer->status == 200)
            {
                LogModule::INFO("Get Response Success!");
                std::cout<<"The Response Status is:["
                    << answer->status<<"]"<<std::endl;
                std::cout<<"The Response Body is:["
                    << answer->body<<"]"<<std::endl;
            }
            else if(answer->status != 200)
            {
                LogModule::ERROR("Post Get Response Fail!");
                std::string Except("Post Get Response Fail");
                throw Except;
            }
        }
        return answer ;
    }
    Json::Value OllamaProvider::Deserialize(std::string& ResponseString)
    {
        Json::CharReaderBuilder readbuilder ;
        std::unique_ptr<Json::CharReader> reader
            (readbuilder.newCharReader());
        std::string parseerr ;
        Json::Value Response ;
        bool CheckParse = reader->parse(ResponseString.c_str() ,
                              ResponseString.c_str()+ResponseString.size(),
                              &Response, &parseerr);
        if(!CheckParse)
        {
            LogModule::ERROR("Deserialize fail!");
            std::string Except("Deserialize fail");
            throw Except;
        }
        return Response ;
    }
    std::string OllamaProvider::AnalyseForSendMessage(Json::Value Response ,const std::string&& ResponseString)
    {
        if(!Response.empty()&&
             Response.isObject()&&
              (Response.isMember("message"))
        ){if(!Response["message"].empty()&&
               Response["message"].isObject()&&
                Response["message"].isMember(ResponseString)){
                    return Response["message"][ResponseString].asString();
        }
    }
        return "" ;
    }
    std::string OllamaProvider::SendMessages(
        std::vector< Message>& messages,
        std::unordered_map<std::string,std::string>& RequestPrograms
    )    
    {
        IsModelAvailable() ;
        std::string RequestString = Serialize(messages , RequestPrograms , false);
        httplib::Result answer = SendRequestMessage(RequestString);
        Json::Value Response ;
        if(answer != nullptr)
            Response = Deserialize(answer->body);
        else
        {
            LogModule::ERROR("Post Get Response Fail!");
            std::string Except("Post Get Response Fail");
            throw Except;
        }
        std::string ret_1 = AnalyseForSendMessage(Response ,"content");
        if( ret_1 != "") return ret_1 ;
        std::string ret_2 = AnalyseForSendMessage(Response ,"thinking");
        if( ret_2 != "") return ret_2 ;
        return "" ;
    }
    std::string OllamaProvider::AnalyseForSendMessageStream(
        Json::Value Response ,
        const std::string&& ResponseString
    )
    {
        if(!Response.empty()&&
             Response.isObject()&&
              (Response.isMember(ResponseString))
        ){
            //{"response":" a","done":false}
            if(ResponseString == "done")
                return Response[ResponseString].asBool() ? "true" : "false";
            else if(ResponseString == "message")
                return Response[ResponseString]["content"].asString();
        }
        return "" ;
    }
    std::string OllamaProvider::SendMessagesAsStream(
        std::vector< Message>& messages,
        std::unordered_map<std::string, std::string>& RequestPrograms,
        func_t callback
    )    
    {
        //检查模型有没有起来
        IsModelAvailable();
        //构建请求报文的主体部分
        std::string RequestBodyString = Serialize(messages , RequestPrograms, true);
        //客户端，流式请求，发送等待可以长一点
        httplib::Client client = CreateClient(60 , 300); 
        //报头信息
        httplib::Headers TheHanders = {
            {"Content-Type" , "application/json"},
            {"Accept" , "text/event-stream"}
        };
        
        httplib::Request request ;
        request.method = "POST" ;
        request.path = "/api/chat" ;
        request.headers = TheHanders ;
        request.body = RequestBodyString ;

        bool ERROR_STATUS = false ;
        std::string ERROR_DESCRIPTION = "" ;
        std::string AllResponse = "" ;
        std::string buffer ;
        bool StringEndERR = true ;

        request.response_handler = [&](const httplib::Response &response)->bool{
            if(response.status != 200)
            {
                ERROR_STATUS = true;
                ERROR_DESCRIPTION = 
                    "发送出去了，收到的是错误的,错误码:"+ 
                        std::to_string(response.status);
                LogModule::ERROR(ERROR_DESCRIPTION);
                return false ;
            }
            return true ;
        };
        request.content_receiver = [&](
            const char *data, 
            size_t len, 
            size_t offset, 
            size_t alllen
        )->bool{
            if(ERROR_STATUS)
                return false ;
            std::string RecvBuffer(data,len);
            buffer += RecvBuffer ;
            
            std::string Pos_Ollama = "\n";
            size_t sep_message = buffer.find(Pos_Ollama);
            while(sep_message != std:: string:: npos)
            {
                std::string trunk ;
                trunk = buffer.substr(0 , sep_message);
                buffer.erase(0,sep_message + Pos_Ollama.size());
                //反序列化:
                Json::Value DataJson ;
                std::string Error ;
                Json::CharReaderBuilder builder ;
                std::unique_ptr<Json::CharReader> reader
                        (builder.newCharReader());
                
                int check = reader->parse(
                    trunk.c_str(),
                    trunk.c_str() + trunk.size(),
                    &DataJson,
                    &Error 
                );
                if(!check)
                {
                    LogModule::ERROR("Deserialize fail!" + Error);
                    std::string Except("Deserialize fail");
                    throw Except;
                }
                
                std::string ret_1 = AnalyseForSendMessageStream(DataJson ,"message");
                if( ret_1 != "")
                {
                    AllResponse += ret_1 ;
                    callback(ret_1 , false);
                }
                else LogModule::ERROR("Analyse The String" + Error);
                std::string ret_2 = AnalyseForSendMessageStream(DataJson ,"done");
                if( ret_2 != "") 
                {
                    if(ret_2 == "true")
                    {
                        StringEndERR = false ;
                        return false ;
                    }
                }
                else LogModule::ERROR("Analyse The String" + Error);
                sep_message = buffer.find(Pos_Ollama);
            }
            return true ;
        };
        bool result = client.send(request);
        if(result == false)
        {
            //报文根本没有发出去！
            if(ERROR_STATUS)
            {
                LogModule::CRITICAL("状态错误!");
                callback("" , true);
                return "";
            }
        }
        if(StringEndERR)
        {
            LogModule::ERROR("接收的报文异常结束!");
            callback("" , true);
        }
        return AllResponse ;
    }
}