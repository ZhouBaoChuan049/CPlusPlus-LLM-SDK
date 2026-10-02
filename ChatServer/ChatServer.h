#include <ChatSDK.h>
#include "../ChatSDK/Third_Party/Httplib/httplib.h"
//三方依赖，我本来想把它放在SDK目录外面的，想来想去放在里面更合理一点。
using namespace Cplusplus_LLM_Provider ;
namespace ChatServerModule
{
    class ChatServer
    {
    public:
        ChatServer()
            :_isrunning(false),
             _chatsdk(nullptr),
             _chatserver(nullptr)
        {}
        bool InitChatServer();
        bool Start();
        bool Stop();
        ~ChatServer() = default ;
    private:

    private:
        std::shared_ptr<Cplusplus_LLM_Provider::ChatSDK> _chatsdk ;
        std::shared_ptr<httplib::Server> _chatserver ;
        bool _isrunning ;
    };
}