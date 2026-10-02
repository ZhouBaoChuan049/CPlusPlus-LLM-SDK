#ifndef __SESSION_MANAGER__
#define __SESSION_MANAGER__
#include "Common.h"
#include "CommonStruct.h"
#include "DataManager.h"
namespace Cplusplus_LLM_Provider
{
    class SessionManager
    {
    public:
        SessionManager(std::string dbname);
        ~SessionManager() = default ;
        std::string CreatSession(const std::string& SessionName,const std::string ModelName, const std::string userName = "");
        std::shared_ptr<Session> GetSession(const std::string& SessionId);
        bool AddMessage(const std::string SessionId , const Message& message);
        std::vector<Message> GetHistoryMessages(const std::string SessionId);
        void UpdateSessionTimesTamp(const std::string& SessionId);
        std::vector<std::string> GetSessionLists() const;
        std::vector<std::shared_ptr<Session>> GetSessionsByUser(const std::string& userName) const;
        bool DeleteSession(const std::string& SessionId);
        void ClearAllSessions();
        size_t GetSessionCount() const;
    private:
        std::string GenerateSessionId();
        std::string GenerateMessageId(size_t MessageCounter);
        std::unordered_map<std::string,std::shared_ptr<Session>> _sessions;
        mutable std::mutex _mutex;
        std::atomic<int64_t> _SessionCount = {0};
        DataManager _dataManager ;
    };
}
#endif