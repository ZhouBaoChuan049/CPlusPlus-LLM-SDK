#include "../Include/UserManager.h"
#include "../Include/Util/LogModule.h"
namespace Cplusplus_LLM_Provider
{
    UserManager::UserManager(std::string dbname)
        :_dataManager(dbname)
    {
        LoadAllUsers();
    }
    void UserManager::LoadAllUsers()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto users = _dataManager.getAllUsers();
        for(const auto& user : users)
            _users[user->_UserName] = user;
        LogModule::INFO("从数据库加载用户数据{}条成功!", users.size());
    }
    std::shared_ptr<User> UserManager::FindUser(const std::string& username)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _users.find(username);
        if(it != _users.end())
            return it -> second;
        //内存中没有，尝试从数据库查找
        auto user = _dataManager.getUser(username);
        if(user != nullptr)
            _users[username] = user;
        return user;
    }
    bool UserManager::AddUser(const std::string& username, const std::string& passwordHash, const std::string& salt)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_users.find(username) != _users.end())
        {
            LogModule::ERROR("用户{}已存在,不能重复添加!",username);
            return false;
        }
        if(!_dataManager.insertUser(username,passwordHash,salt))
        {
            LogModule::ERROR("用户{}写入数据库失败!",username);
            return false;
        }
        auto user = std::make_shared<User>(username,passwordHash,salt);
        _users[username] = user;
        LogModule::INFO("用户{}注册成功!",username);
        return true;
    }
    std::vector<std::shared_ptr<Session>> UserManager::GetUserSessions(const std::string& username)
    {
        return _dataManager.getSessionsByUser(username);
    }
}