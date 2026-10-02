#ifndef __USER_MANAGER__
#define __USER_MANAGER__
#include "Common.h"
#include "CommonStruct.h"
#include "DataManager.h"
namespace Cplusplus_LLM_Provider
{
    class UserManager
    {
    public:
        UserManager(std::string dbname);
        ~UserManager() = default;
        //查找用户，返回完整用户对象；不存在则返回nullptr
        std::shared_ptr<User> FindUser(const std::string& username);
        //新增用户，成功返回true
        bool AddUser(const std::string& username, const std::string& passwordHash, const std::string& salt);
        //获取某用户的历史会话（通过DataManager查询）
        std::vector<std::shared_ptr<Session>> GetUserSessions(const std::string& username);
    private:
        void LoadAllUsers();
    private:
        std::unordered_map<std::string,std::shared_ptr<User>> _users;  //用户名 -> 用户对象
        mutable std::mutex _mutex;
        DataManager _dataManager;
    };
}
#endif