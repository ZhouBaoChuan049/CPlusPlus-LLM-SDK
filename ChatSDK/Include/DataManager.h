#ifndef __DATA_MANAGER__
#define __DATA_MANAGER__

#include "Common.h"
#include "CommonStruct.h"
#include "sqlite3.h"
namespace Cplusplus_LLM_Provider
{
    class DataManager
    {
    public:
        DataManager(std::string dbname);
        ~DataManager();
        bool InsertSession(const Session& session);
        std::shared_ptr<Session> GetSession(const std::string& sessionId)const;
        bool updateSessionTimestamp(const std::string& sessionId, std::time_t timestamp);
        bool deleteSession(const std::string& sessionId);
        std::vector<std::string> getAllSessionIds()const;
        std::vector<std::shared_ptr<Session>> getAllSessions()const;
        std::vector<std::shared_ptr<Session>> getSessionsByUser(const std::string& username)const;
        bool clearAllSessions();
        size_t getSessionCount()const;


        bool insertMessage(const std::string& sessionId, const Message& message);
        std::vector<Message> getSessionMessages(const std::string& sessionId)const;
        bool deleteSessionMessages(const std::string& sessionId);

        //用户数据
        bool insertUser(const std::string& username, const std::string& passwordHash, const std::string& salt);
        std::shared_ptr<User> getUser(const std::string& username)const;
        std::vector<std::shared_ptr<User>> getAllUsers()const;

    private:
        bool InitDataBase();
    private:
        sqlite3* _db ;
        std::string _name;
        mutable std::mutex _mutex;
    };
}

#endif