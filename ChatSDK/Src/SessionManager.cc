#include "../Include/SessionManager.h"
#include "../Include/Common.h"
#include "../Include/CommonStruct.h"
#include "../Include/Util/LogModule.h"
namespace Cplusplus_LLM_Provider
{
    SessionManager::SessionManager(std::string dbname)
        :_dataManager(dbname)
    {
        //第二次及以后打开我这个数据库的时候，就会去把数据库里的数据全部读取上来放到内存中。
        std::vector<std::string> ids = 
            _dataManager.getAllSessionIds();
        for(int i = 0 ; i < ids.size() ; i ++)
            _sessions.insert(make_pair(ids[i] , _dataManager.GetSession(ids[i]))); 
    }
    //需要在持有_mutex的情况下调用
    std::string SessionManager::GenerateSessionId()
    {
        int64_t count = _SessionCount.fetch_add(1) + 1;
        time_t time = std::time(nullptr);
        std::ostringstream oss ;
        oss<<"Session_"<<time<<"_"<<std::setw(8)<<
            std::setfill('0')<<count; 
        return oss.str();
    }
    //需要在持有_mutex的情况下调用
    std::string SessionManager::GenerateMessageId(size_t MessageCounter)
    {
        MessageCounter++ ;
        time_t time = std::time(nullptr);
        std::ostringstream oss ;
        oss<<"Session_"<<time<<"_"<<std::setw(8)<<
            std::setfill('0')<<MessageCounter; 
        return oss.str();
    }

    std::string SessionManager::CreatSession
        (const std::string& SessionName,const std::string ModelName, const std::string userName)
    {
        Session session(SessionName, userName) ;
        session._TimeCreate = std::time(nullptr);
        session._LastTime = std::time(nullptr);
        session._ModelNameUsed = ModelName ;
        std::lock_guard<std::mutex> lock(_mutex);
        session._SessionID = GenerateSessionId();
        //维护在内存里面
        _sessions[session._SessionID] = std::make_shared<Session>(session) ;
        //在数据库里面也维护一份
        _dataManager.InsertSession(session);
        return session._SessionID ;
    }
    std::shared_ptr<Session> SessionManager::GetSession(const std::string& SessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(SessionId);
        if(it != _sessions.end())
        {
            LogModule::INFO("在内存中找到了对应的会话!");
            return it -> second ;
        }
        //没有在内存中找到，就去数据库里面找
        return _dataManager.GetSession(SessionId);
    }
    bool SessionManager::AddMessage(const std::string SessionId , 
            const Message& message)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::shared_ptr<Session> TheSession = nullptr ;
        auto it = _sessions.find(SessionId);
        if(it != _sessions.end())
            TheSession = it -> second ;
        else
            TheSession = _dataManager.GetSession(SessionId);
        if(TheSession == nullptr)
        {
            LogModule::ERROR("对会话{}添加消息失败!",SessionId);
            return false ;
        }
        Message NewMessage(message._Role,message._Content);
        NewMessage._MessageID = GenerateMessageId(TheSession->_Messages.size());
        NewMessage._Time = std::time(nullptr);
        TheSession->_Messages.push_back(NewMessage);

        _dataManager.insertMessage(SessionId,NewMessage);
        return true ;
    }
    std::vector<Message> SessionManager::GetHistoryMessages(const std::string SessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(SessionId);
        if(it != _sessions.end())
        {
            LogModule::INFO("在内存中获取历史消息成功!");
            return it -> second -> _Messages ;
        }
        return _dataManager.getSessionMessages(SessionId);
    }
    void SessionManager::UpdateSessionTimesTamp(const std::string& SessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(SessionId);
        if(it != _sessions.end())
        {
            it->second->_LastTime = std::time(nullptr);
            LogModule::INFO("在内存中找到了会话信息");
        }
        _dataManager.updateSessionTimestamp(SessionId,std::time(nullptr));
    }
    std::vector<std::string> SessionManager::GetSessionLists() const
    {
        //其实一般性来说所有的Session都会在内存里面.
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> lists ;
        for(auto it : _sessions)
            lists.push_back(it.first);
        return lists;
    }
    std::vector<std::shared_ptr<Session>> SessionManager::GetSessionsByUser(const std::string& userName) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::shared_ptr<Session>> result;
        for(const auto& it : _sessions)
            if(it.second->_UserName == userName)
                result.push_back(it.second);
        return result;
    }
    bool SessionManager::DeleteSession(const std::string& SessionId)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(SessionId);
        if(it == _sessions.end())
        {
            LogModule::ERROR("删除会话{}失败!",SessionId);
            return false ;
        }
        _sessions.erase(it);
        _dataManager.deleteSession(SessionId);
        return true;
    }
    void SessionManager::ClearAllSessions()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _sessions.clear();
        _dataManager.clearAllSessions();
    }
    size_t SessionManager::GetSessionCount() const
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if(_dataManager.getSessionCount() != _sessions.size())
        {
            LogModule::ERROR("数据库和内存的会话同步异常!\
                数据库会话总数:{},内存会话总数:{}",
                _dataManager.getSessionCount(),_sessions.size());
            return 0 ;
        }
        return _sessions.size();
    }
}
