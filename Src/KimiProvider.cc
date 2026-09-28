#include "../Include/Common.h"
#include "../Include/Util/LogModule.h"
#include "../Include/KimiProvider.h"
#include "../Include/CommonStruct.h"

namespace Cplusplus_LLM_Provider
{

bool KimiProvider::IsModelAvailable()
{
    if(!GetAvailable())
    {
        LogModule::CRITICAL("Kimi Model is not available!");
        std::string Excepts("Kimi Model is not available");
        throw Excepts;
    }

    return true;
}

void KimiProvider::InitModel(
    std::unordered_map<std::string,std::string> Config)
{
    if(Config.find("_ApiKey") == Config.end())
    {
        LogModule::CRITICAL(
            "InitModel Fail!{}",
            "_ApiKey NoFind"
        );
        exit(Exception::INIT_EER);
    }
    else
    {
        SetApiKey(Config["_ApiKey"]);
    }

    if(Config.find("_APIAccessAddress") == Config.end())
    {
        LogModule::CRITICAL(
            "InitModel Fail!{}",
            "_APIAccessAddress NoFind"
        );
        exit(Exception::INIT_EER);
    }
    else
    {
        SetAPIAccessAddress(Config["_APIAccessAddress"]);
    }

    SetAvailable(true);
}

std::string KimiProvider::GetModelName()
{
    return "Kimi K3";
}

ModelInfo KimiProvider::GetModelDescription()
{
    std::string Description =
        "Kimi K3是月之暗面(Moonshot AI)推出的旗舰级、"
        "开放权重、超长上下文、重点强化 Coding 和 Agent 能力的大模型";

    ModelInfo info(
        GetModelName(),
        Description,
        GetAPIAccessAddress()
    );

    info._IsThisModelAvailable = GetAvailable();

    return info;
}

std::string KimiProvider::Serialize(
    std::vector<Message>& messages,
    std::unordered_map<std::string,std::string>& RequestPrograms,
    bool isstream)
{
    std::string model = "kimi-k3";
    bool stream = isstream;
    int Max_token = 0;
    if(RequestPrograms.find("Max_token") != RequestPrograms.end())
    {
        Max_token = std::stoi(RequestPrograms["Max_token"]);
    }
    Json::Value BodyValues;
    for(auto message : messages)
    {
        Json::Value value;

        value["role"] = message._Role;
        value["content"] = message._Content;
        BodyValues.append(value);
    }
    Json::Value RequestBody;
    RequestBody["model"] = model;
    RequestBody["messages"] = BodyValues;
    if(Max_token > 0)
        RequestBody["max_completion_tokens"] = Max_token;
    RequestBody["stream"] = stream;
    if(RequestPrograms.find("reasoning_effort") != RequestPrograms.end())
    {
        RequestBody["reasoning_effort"] =
            RequestPrograms["reasoning_effort"];
    }
    if(RequestPrograms.find("prompt_cache_key")
        != RequestPrograms.end())
    {
        RequestBody["prompt_cache_key"] =
            RequestPrograms["prompt_cache_key"];
    }
    Json::StreamWriterBuilder builder;

    std::unique_ptr<Json::StreamWriter> writer(
        builder.newStreamWriter()
    );
    std::ostringstream ss;
    int CheckWrite = writer->write(
        RequestBody,
        &ss
    );
    if(CheckWrite != 0)
    {
        LogModule::ERROR("Json Writer fail!");

        std::string Excepts("Json Writer fail");
        throw Excepts;
    }
    return ss.str();
}

httplib::Client KimiProvider::CreateClient(
    int commect_timeout,
    int read_timeout)
{
    httplib::Client client(
        GetAPIAccessAddress()
    );
    client.set_connection_timeout(
        commect_timeout,
        0
    );
    client.set_read_timeout(
        read_timeout,
        0
    );
    return client;
}

httplib::Result KimiProvider::SendRequestMessage(
    std::string& RequestBodyString)
{
    httplib::Client client = CreateClient(30,60);
    httplib::Headers Headers = {
        {
            "Content-Type",
            "application/json"
        },
        {
            "Authorization",
            "Bearer " + GetApiKey()
        }
    };
    httplib::Result answer = client.Post(
        "/v1/chat/completions",
        Headers,
        RequestBodyString,
        "application/json"
    );

    if(answer == nullptr)
    {
        std::cerr
            << "HTTP Request Error: "
            << httplib::to_string(answer.error())
            << std::endl;
        LogModule::ERROR("Post Request Failed!");
        return answer;
    }

    if(answer->status == 200)
    {
        LogModule::INFO("Get Response Success!");
        std::cout
            << "The Response Status is:["
            << answer->status
            << "]"
            << std::endl;

        std::cout
            << "The Response Body is:["
            << answer->body
            << "]"
            << std::endl;
    }
    else
    {
        LogModule::ERROR("Post Get Response Fail!");

        std::string Except(
            "Post Get Response Fail, status:"
            + std::to_string(answer->status)
        );

        throw Except;
    }

    return answer;
}

Json::Value KimiProvider::Deserialize(
    std::string& ResponseString)
{
    Json::CharReaderBuilder readbuilder;

    std::unique_ptr<Json::CharReader> reader(
        readbuilder.newCharReader()
    );

    std::string parseerr;

    Json::Value Response;

    bool CheckParse = reader->parse(
        ResponseString.c_str(),
        ResponseString.c_str() + ResponseString.size(),
        &Response,
        &parseerr
    );

    if(!CheckParse)
    {
        LogModule::ERROR(
            "Deserialize fail! {}",
            parseerr
        );

        std::string Except("Deserialize fail");
        throw Except;
    }

    return Response;
}

std::string KimiProvider::SendMessages(
    std::vector<Message>& messages,
    std::unordered_map<std::string,std::string>& RequestPrograms)
{
    IsModelAvailable();
    std::string RequestBodyString =
        Serialize(
            messages,
            RequestPrograms,
            false
        );
    httplib::Result answer =
        SendRequestMessage(
            RequestBodyString
        );
    Json::Value Response;
    if(answer != nullptr)
    {
        Response = Deserialize(
            answer->body
        );
    }
    else
    {
        LogModule::ERROR(
            "Post Get Response Fail!"
        );

        std::string Except(
            "Post Get Response Fail"
        );

        throw Except;
    }

    if(Response.isMember("choices") &&
       Response["choices"].isArray() &&
       !Response["choices"].empty())
    {
        for(int i = 0;
            i < static_cast<int>(Response["choices"].size());
            i++)
        {
            if(Response["choices"][i].isMember("message") &&
               Response["choices"][i]["message"].isObject())
            {
                Json::Value MessageValue =
                    Response["choices"][i]["message"];
                if(MessageValue.isMember("content") &&
                   MessageValue["content"].isString())
                {
                    return MessageValue["content"].asString();
                }
            }
        }
    }
    if(Response.isMember("error"))
    {
        if(Response["error"].isMember("message") &&
           Response["error"]["message"].isString())
        {
            std::string Except =
                "Kimi API Error: "
                + Response["error"]["message"].asString();

            throw Except;
        }
    }

    return "";
}

std::string KimiProvider::SendMessagesAsStream(
    std::vector<Message>& messages,
    std::unordered_map<std::string, std::string>& RequestPrograms,
    func_t callback)
{
    IsModelAvailable();
    std::string RequestBodyString =
        Serialize(
            messages,
            RequestPrograms,
            true
        );
    httplib::Client client =
        CreateClient(
            60,
            300
        );

    httplib::Headers TheHeaders = {
        {
            "Content-Type",
            "application/json"
        },
        {
            "Authorization",
            "Bearer " + GetApiKey()
        },
        {
            "Accept",
            "text/event-stream"
        }
    };

    httplib::Request request;
    request.method = "POST";
    request.path ="/v1/chat/completions";
    request.headers =TheHeaders;
    request.body =RequestBodyString;
    bool ERROR_STATUS = false;
    std::string ERROR_DESCRIPTION = "";
    std::string AllResponse = "";
    std::string buffer;
    bool StringEndERR = true;

    request.response_handler =
        [&](const httplib::Response &response)->bool
    {
        if(response.status != 200)
        {
            ERROR_STATUS = true;

            ERROR_DESCRIPTION =
                "发送出去了，收到的是错误的,错误码:"
                + std::to_string(response.status);

            LogModule::ERROR(
                ERROR_DESCRIPTION
            );

            return false;
        }

        return true;
    };

    request.content_receiver =
        [&](
            const char *data,
            size_t len,
            size_t offset,
            size_t alllen
        )->bool
    {
        if(ERROR_STATUS)
            return false;
        std::string RecvBuffer(
            data,
            len
        );

        buffer += RecvBuffer;

        size_t sep_message =
            buffer.find(POS);

        while(sep_message != std::string::npos)
        {
            std::string trunk =
                buffer.substr(
                    0,
                    sep_message
                );

            buffer.erase(
                0,
                sep_message + POS.size()
            );
            size_t sep_word =
                trunk.find("data:");

            if(sep_word == std::string::npos)
            {
                sep_message =
                    buffer.find(POS);

                continue;
            }
            size_t DataStart =
                sep_word + 5;

            if(DataStart < trunk.size() &&
               trunk[DataStart] == ' ')
            {
                DataStart++;
            }

            std::string DataString =
                trunk.substr(
                    DataStart
                );
            if(DataString == "[DONE]")
            {
                LogModule::INFO(
                    "响应报文读取正常结束![DONE]"
                );

                StringEndERR = false;

                AllResponse += "[DONE]";

                callback(
                    "[DONE]",
                    true
                );

                return false;
            }
            Json::Value DataJson;

            std::string Error;

            Json::CharReaderBuilder builder;

            std::unique_ptr<Json::CharReader> reader(
                builder.newCharReader()
            );

            int check =
                reader->parse(
                    DataString.c_str(),
                    DataString.c_str()
                        + DataString.size(),
                    &DataJson,
                    &Error
                );

            if(!check)
            {
                LogModule::ERROR(
                    "Deserialize fail!{}",
                    Error
                );

                std::string Except(
                    "Deserialize fail"
                );

                throw Except;
            }

            if(DataJson.isObject() &&
               DataJson.isMember("choices") &&
               DataJson["choices"].isArray() &&
               !DataJson["choices"].empty())
            {
                Json::Value Choice =
                    DataJson["choices"][0];

                if(Choice.isObject() &&
                   Choice.isMember("delta") &&
                   Choice["delta"].isObject())
                {
                    Json::Value Delta =
                        Choice["delta"];
                    if(Delta.isMember("content") &&
                       Delta["content"].isString())
                    {
                        std::string Content =
                            Delta["content"].asString();

                        if(!Content.empty())
                        {
                            AllResponse += Content;

                            callback(
                                Content,
                                false
                            );
                        }
                    }
                }
            }

            sep_message =
                buffer.find(POS);
        }

        return true;
    };

    bool result =
        client.send(request);

    if(result == false)
    {
        if(ERROR_STATUS)
        {
            LogModule::CRITICAL(
                "Kimi API 状态错误!"
            );

            callback(
                "",
                true
            );

            return "";
        }

        LogModule::ERROR(
            "Kimi API Request Failed!"
        );

        callback(
            "",
            true
        );

        return "";
    }

    if(StringEndERR)
    {
        LogModule::ERROR(
            "接收的报文在没有收到[DONE]的情况下异常结束!"
        );

        callback(
            "",
            true
        );
    }

    return AllResponse;
}
}