#include "../Include/ChatGPTProvider.h"
#include "../Include/Util/LogModule.h"
#include "../Include/Common.h"
#include "../Include/CommonStruct.h"

namespace Cplusplus_LLM_Provider
{
    bool ChatGPTProvider::IsModelAvailable()
    {
        return GetAvailable() ;
    }
    void ChatGPTProvider::InitModel
        (std::shared_ptr<Config> Config)
    {
        auto apiConfig = std::dynamic_pointer_cast<APIConfig>(Config);
        if(!apiConfig)
        {
            LogModule::CRITICAL("InitModel Fail!{}","Config 类型错误，需要 APIConfig");
            exit(Exception::INIT_EER);
        }
        SetApiKey(apiConfig->_apiKey);
        SetAPIAccessAddress("https://leapone.leapinfra.cn");
        SetAvailable(true) ;
    }
    std::string ChatGPTProvider::GetModelName() 
    {
        std::string name = "gpt-5.5";
        return name;
    }
    ModelInfo ChatGPTProvider::GetModelDescription()
    {
        std::string Description = 
        "GPT-5.5 是 OpenAI 的新一代通用大语言模型，具备较强的复杂推理、\
        代码生成、文本理解与任务执行能力。适用于软件开发、技术问答、内容生成、\
        数据分析以及多步骤复杂任务等场景。" ;
        ModelInfo info(
            GetModelName(),
            Description,
            GetAPIAccessAddress()
        ) ;
        info._IsThisModelAvailable = GetAvailable();
        return info ;
    }
    std::string ChatGPTProvider::SendMessages(
        std::vector<Message>& messages,
        std::unordered_map<std::string,std::string>& RequestPrograms
    )
    {
        if(!IsModelAvailable()) {
            LogModule::CRITICAL("致命错误!模型不可用。");
            exit(INIT_EER);
        }
        //我们要构建整个请求报文，包括请求体（有历史请求报文和请求参数构成）
        //以及一个请求头，包含三个参数，比Deepseek多一个代理参数。
        Json::Value BodyMessages ;
        for(auto message : messages){
            Json::Value msg ;
            msg["role"] = message._Role ;
            msg["content"] = message._Content ;
            BodyMessages.append(msg);
        }
        std::string _model = "gpt-5.5";
        // if(RequestPrograms.find("model") != RequestPrograms.end())
        //     _model = RequestPrograms["model"];
        double _temperature ;
        if(RequestPrograms.find("temperature") != RequestPrograms.end())
            _temperature = std::stod(RequestPrograms["temperature"]);
        int _max_output_tokens ;
        if(RequestPrograms.find("max_output_tokens") != RequestPrograms.end())
            _max_output_tokens = std::stoi(RequestPrograms["max_output_tokens"]);

        //把报文序列化
        Json::Value RequestBody ;
        RequestBody["messages"] = BodyMessages ; 
        //RequestBody["input"] = BodyMessages ; //走中转不能用这个
        RequestBody["model"] = _model ;
        RequestBody["temperature"] = _temperature ;
        //RequestBody["max_output_tokens"] = _max_output_tokens ; //走中转不能用这个
        RequestBody["max_completion_tokens"] = _max_output_tokens ; 
        Json::StreamWriterBuilder builder ;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        std::ostringstream ss ;
        int check = writer->write(RequestBody , &ss);
        if(check != 0){
            LogModule::ERROR("错误!请求报文的主体部分序列化失败。");
            std::string Except = "请求报文的主体部分序列化失败" ;
            throw Except ;
        }
        std::string RequestBodyString = ss.str();
        std::cout<< RequestBodyString <<std::endl;
        httplib::Client client(GetAPIAccessAddress());
        client.set_connection_timeout(60,0);
        client.set_read_timeout(120,0);
        //client.set_proxy("172.17.112.1", 10090); // 中转站是国内的可能不需要代理？我要代理可能会出错？
        
        httplib::Headers handers = {
            {"Content-Type" , "application/json"},
            {"Authorization" , "Bearer " + GetApiKey() }
        };
        httplib::Result result = client.Post(
            "/v1/chat/completions", //这里也是走中转站
            handers, 
            RequestBodyString,
            "application/json"
        );///chat/completions
        if(result == nullptr){
            LogModule::ERROR("没有接收到报文啊!");
            std::string Except("没有接收到报文啊!");
            throw Except ; 
        }
        else{
            if(result->status != 200){
                LogModule::ERROR("HTTP状态码: {}", result->status);
                LogModule::ERROR("服务器返回: {}", result->body);
                return "";
            }
            else
                LogModule::INFO("拿到报文了,报文状态200!");
        }
        std::string ResponseString = result->body ;
        Json::Value ResponseJson ;
        std::string errstr ;
        Json::CharReaderBuilder readbuilder ;
        std::unique_ptr<Json::CharReader> reader(readbuilder.newCharReader());
        bool ans = reader->parse(
            ResponseString.c_str(),
            ResponseString.c_str() + ResponseString.size(),
            &ResponseJson,
            &errstr
        );
        if(!ans)
        {
            LogModule::ERROR("错误!应答报文反序列化错误!");
            std::string Except("错误!应答报文反序列化错误!");
            throw Except ; 
        }
        std::cout<<ResponseJson.toStyledString()<<std::endl;
        if(ResponseJson.isObject() &&
            ResponseJson.isMember("choices") &&
            !ResponseJson.empty()){
            if(ResponseJson["choices"].isArray() &&
                !ResponseJson["choices"].empty()){
                if(!ResponseJson["choices"][0].empty() &&
                    ResponseJson["choices"][0].isObject() &&
                    ResponseJson["choices"][0].isMember("message")){
                    if(ResponseJson["choices"][0]["message"].isObject() &&
                        !ResponseJson["choices"][0]["message"].empty() &&
                        ResponseJson["choices"][0]["message"].isMember("content")){
                        if(ResponseJson["choices"][0]["message"]["content"].isString()){
                            return ResponseJson["choices"][0]["message"]["content"].asString();
                        }
                    }
                }
            }
        }
        LogModule::ERROR("报文解析失败!");
        return "";
    }


    std::string ChatGPTProvider::SendMessagesAsStream(
        std::vector<Message>& messages,
        std::unordered_map<std::string, std::string>& RequestPrograms,
        func_t callback
    )
    {
        if(!IsModelAvailable()) {
            LogModule::CRITICAL("致命错误!模型不可用。");
            exit(INIT_EER);
        }
        Json::Value BodyMessages ;
        for(auto message : messages){
            Json::Value msg ;
            msg["role"] = message._Role ;
            msg["content"] = message._Content ;
            BodyMessages.append(msg);
        }
        std::string _model = "gpt-5.5";
        // if(RequestPrograms.find("model") != RequestPrograms.end())
        //     _model = RequestPrograms["model"];
        double _temperature ;
        if(RequestPrograms.find("temperature") != RequestPrograms.end())
            _temperature = std::stod(RequestPrograms["temperature"]);
        int _max_output_tokens ;
        if(RequestPrograms.find("max_output_tokens") != RequestPrograms.end())
            _max_output_tokens = std::stoi(RequestPrograms["max_output_tokens"]);

        //把报文序列化
        Json::Value RequestBody ;
        RequestBody["messages"] = BodyMessages ;
        RequestBody["model"] = _model ;
        RequestBody["temperature"] = _temperature ;
        RequestBody["max_completion_tokens"] = _max_output_tokens ; 
        RequestBody["stream"] = true ;
        Json::StreamWriterBuilder builder ;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        std::ostringstream ss ;
        int check = writer->write(RequestBody , &ss);
        if(check != 0){
            LogModule::ERROR("错误!请求报文的主体部分序列化失败。");
            std::string Except = "请求报文的主体部分序列化失败" ;
            throw Except ;
        }
        std::string RequestBodyString = ss.str();
        std::cout<<RequestBodyString<<std::endl;

        httplib::Client client(GetAPIAccessAddress());
        client.set_connection_timeout(60,0);
        client.set_read_timeout(120,0);
        client.set_proxy("172.17.112.1", 10090);
        
        httplib::Headers _handers = {
            {"Content-Type" , "application/json"},
            {"Authorization" , "Bearer " + GetApiKey() },
            {"Accept" , "text/event-stream"}
        };    

        std::string ERROR_DESCRIPTION ="";
        bool ERROR_STATUS =false ;
        std::string buffer = "" ;
        bool StringEndERR = false ;
        std::string AllResponse = "" ;

        httplib::Request request ;
        request.method = "POST" ;
        request.path = "/v1/chat/completions";
        request.headers = _handers ;
        request.body = RequestBodyString ;
        request.response_handler = 
            [&](const httplib::Response& response)->bool{
                std::cout << "HTTP Status = "
                    << response.status<< std::endl;
            if(response.status != 200)
            {
                //debug
                std::cout<< response.body<<std::endl;
                ERROR_STATUS = true;
                ERROR_DESCRIPTION = 
                    "发送出去了，收到的是错误的,错误码:"+ 
                        std::to_string(response.status);
                LogModule::ERROR(ERROR_DESCRIPTION);
                //return false ;
            }
            return true ;
        };
        std::string ERROR_BODY = "";
        request.content_receiver = [&](
            const char *data, 
            size_t len, 
            size_t offset, 
            size_t alllen
        )->bool{
            if(ERROR_STATUS)
            {
                ERROR_BODY.append(data, len);
                return false ;
            }

            //从接收缓冲区读上来
            std::string RecvBuffer(data,len);
            buffer += RecvBuffer ;
                        size_t sep_message = buffer.find(POS);
            while(sep_message != std:: string:: npos)
            {
                std::string trunk ;
                trunk = buffer.substr(0 , sep_message);
                buffer.erase(0,sep_message + POS.size());
                //我能保证我读取到的报文是完整的.
                size_t sep_word = trunk.find("data:");

                if(sep_word == std::string::npos)
                {
                    sep_message = buffer.find(POS);
                    continue ;
                }
                size_t end_point = trunk.find(
                    "\n" , 
                    sep_word 
                ); 
                // std::cout << "trunk = [" << trunk << "]" << std::endl;
                // std::cout << "sep_word = " << sep_word << std::endl;
                // std::cout << "end_point = " << end_point << std::endl;
                std::string DataString = trunk.substr(
                    sep_word + 5 + (trunk[sep_word+5] == ' ' ? 1 : 0)
                );
                //如果这个data是整个报文的最后一个字段，那么这个data后面就不会有\n，进一步判断是不是[DONE]
                if(end_point == std::string::npos && DataString == "[DONE]")
                {
                    //可能是[DONE]
                    LogModule::INFO("响应报文读取正常结束![DONE]");
                    StringEndERR = false ;
                    AllResponse += "[DONE]";
                    callback("[DONE]",true);
                    return false ;
                }
                else
                {
                    //如果这个data是整个报文的最后一个字段并且不是[DONE]走这里。
                    //那么我想，照理说是不需要再次分割的，但是这段逻辑是考虑到data不是整个报文的最后一个字段的时候。
                    DataString = trunk.substr(
                        sep_word + 5 + (trunk[sep_word+5] == ' ' ? 1 : 0),//跳过空格
                        end_point - sep_word - 6
                    );
                    //反序列化:
                    Json::Value DataJson ;
                    std::string Error ;
                    Json::CharReaderBuilder builder ;
                    std::unique_ptr<Json::CharReader> reader
                            (builder.newCharReader());
                    //debug:
                    std::cout<<DataString<<std::endl;
                    
                    int check = reader->parse(
                        DataString.c_str(),
                        DataString.c_str() + DataString.size(),
                        &DataJson,
                        &Error 
                    );//data: [DONE]
                    if(!check)
                    {
                        LogModule::ERROR("Deserialize fail!" + Error);
                        std::string Except("Deserialize fail");
                        throw Except;
                    }

                    //std::cout<<"[Debug]:"<<DataJson.toStyledString()<<std::endl;
                    if(DataJson.isObject()&&
                    !DataJson.empty()&&
                        DataJson.isMember("choices")){
                        if(DataJson["choices"].isArray()&&
                        !DataJson["choices"].empty()){
                            if(DataJson["choices"][0].isObject()&&
                                DataJson["choices"][0].isMember("delta")){
                                if(DataJson["choices"][0]["delta"].isObject()&&
                                    !DataJson["choices"][0]["delta"].empty()&&
                                        DataJson["choices"][0]["delta"].isMember("content")){
                                            if(DataJson["choices"][0]["delta"]["content"].isString())
                                            {
                                                std::string Content = 
                                                    DataJson["choices"][0]["delta"]["content"].asString() ;
                                                    if(!Content.empty())
                                                    {
                                                        AllResponse += Content;
                                                        //LogModule::DEBUG("有没有成功调用到回调函数");
                                                        callback(Content, false);
                                                    }
                                            }

                                    }
                                }
                        }
                    }
                }
                sep_message = buffer.find(POS);
            }
            return true ;
        };
        //httplib::Result 里面重载了类型转换器.
        bool result = client.send(request);
        if(result == false)
        {
            //报文根本没有发出去！
            if(ERROR_STATUS)
            {
                std::cout<<ERROR_BODY <<std::endl; 
                LogModule::CRITICAL("状态错误!");
                callback("" , true);
                return "";
            }
        }
        if(StringEndERR)
        {
            LogModule::ERROR("接收的报文在没有收到[DONE]的情况下异常结束!");
            callback("" , true);
        }
        return AllResponse ;
            //######################################################################################
            //本来想做事件驱动应答报文的处理机制的，但是找不到合适的OpenAI官方充值渠道，走了中转站，这部分代码块搁置。
            // 读取到bufer了，
            // event: response.output_text.delta
            // data: {"type":"response.output_text.delta","item_id":"msg_001","output_index":0,"content_index":0,"delta":"你好"}
            // 事件驱动类型:
            // response.create：表示响应对象创建好了
            // response.in_progress：表示模型开始工作
            // response.output_text.delta：表示一段段文字实时流出
            // response.output_completed：表示该输出块结束
            // response.completed：表示整个响应结束。
            // int pos = buffer.find(POS);
            // while(pos != std::string:: npos)
            // {
            //     std::string trunk = buffer.substr(0 , pos);
            //     buffer.erase(0 , pos + POS.size());
            //                                           //event: {}
            //     int EventPoint = trunk.find("event:");//012345678
            //     int DataPoint = trunk.find("data:");
            //     int SepPoint = trunk.find('\n',0);
            //     std::string EventString = trunk.substr(EventPoint+7, SepPoint);
            //     std::string DataString = trunk.substr(DataPoint+7);
            //     if(EventString == "response.create")
            //     else if(EventString == "in_progress")
            //     else if(EventString == "output_text.delta")
            //     else if(EventString == "output_completed")
            //     else if(EventString == "completed")
            // }
            //###############################################################################
    }
}