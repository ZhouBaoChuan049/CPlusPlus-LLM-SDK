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
    std::string SessionManager::GenerateSessionId()
    {
        _mutex.lock();
        _SessionCount.fetch_add(1);
        time_t time = std::time(nullptr);
        std::ostringstream oss ;
        oss<<"Session_"<<time<<"_"<<std::setw(8)<<
            std::setfill('0')<<_SessionCount; 
        _mutex.unlock();
        return oss.str();
    }
    std::string SessionManager::GenerateMessageId(size_t MessageCounter)
    {
        _mutex.lock();
        MessageCounter++ ;
        time_t time = std::time(nullptr);
        std::ostringstream oss ;
        oss<<"Session_"<<time<<"_"<<std::setw(8)<<
            std::setfill('0')<<MessageCounter; 
        _mutex.unlock();
        return oss.str();
    }

    std::string SessionManager::CreatSession
        (const std::string& SessionName,const std::string ModelName)
    {
        _mutex.lock();
        Session session(SessionName) ;
        session._TimeCreate = std::time(nullptr);
        session._LastTime = std::time(nullptr);
        session._SessionID = GenerateSessionId();
        session._ModelNameUsed = ModelName ;
        //维护在内存里面
        _sessions[session._SessionID] = std::make_shared<Session>(session) ;
        _mutex.unlock();
        //在数据库里面也维护一份
        _dataManager.InsertSession(session);
        return session._SessionID ;
    }
    std::shared_ptr<Session> SessionManager::GetSession(const std::string& SessionId)
    {
        _mutex.lock();
        auto it = _sessions.find(SessionId);
        if(it != _sessions.end())
        {
            _mutex.unlock();
            LogModule::INFO("在内存中找到了对应的会话!");
            return it -> second ;
        }
        _mutex.unlock();
        //没有在内存中找到，就去数据库里面找
        auto ret = _dataManager.GetSession(SessionId);
        return ret;
    }
    bool SessionManager::AddMessage(const std::string SessionId , 
            const Message& message)
    {
        _mutex.lock();
        std::shared_ptr<Session> TheSession = GetSession(SessionId);
        if(TheSession == nullptr)
        {
            _mutex.unlock();
            LogModule::ERROR("对会话{}添加消息失败!",SessionId);
            return false ;
        }
        Message NewMessage(message._Role,message._Content);
        NewMessage._MessageID = GenerateMessageId(TheSession->_Messages.size());
        NewMessage._Time = std::time(nullptr);
        TheSession->_Messages.push_back(NewMessage);
        _mutex.unlock();

        _dataManager.insertMessage(SessionId,NewMessage);
        return true ;
    }
    std::vector<Message> SessionManager::GetHistoryMessages(const std::string SessionId)
    {
        _mutex.lock();
        std::shared_ptr<Session> TheSession = GetSession(SessionId);
        if(TheSession != nullptr)
        {
            _mutex.unlock();
            LogModule::INFO("在内存中获取历史消息成功!");
            return TheSession->_Messages;
        }
        _mutex.unlock();
        return _dataManager.getSessionMessages(SessionId);
    }
    void SessionManager::UpdateSessionTimesTamp(const std::string& SessionId)
    {
        _mutex.lock();
        auto it = _sessions.find(SessionId);
        if(it != _sessions.end())
        {
            _mutex.unlock();
            it->second->_LastTime = std::time(nullptr);
            LogModule::INFO("在内存中找到了会话信息");
        }
        _mutex.unlock();
        _dataManager.updateSessionTimestamp(SessionId,std::time(nullptr));
        _mutex.unlock();
    }
    std::vector<std::string> SessionManager::GetSessionLists() const
    {
        //其实一般性来说所有的Session都会在内存里面.
        _mutex.lock();
        std::vector<std::string> lists ;
        for(auto it : _sessions)
            lists.push_back(it.first);
        _mutex.unlock();
        return lists;
    }
    bool SessionManager::DeleteSession(const std::string& SessionId)
    {
        _mutex.lock();
        auto it = _sessions.find(SessionId);
        if(it == _sessions.end())
        {
            _mutex.unlock();
            LogModule::ERROR("删除会话{}失败!",SessionId);
            return false ;
        }
        _sessions.erase(it->first);
        _mutex.unlock();

        _dataManager.deleteSession(SessionId);
        return true;
    }
    void SessionManager::ClearAllSessions()
    {
        _mutex.lock();
        for(auto it : _sessions)
        {
            _sessions.erase(it.first);
        }
        _mutex.unlock();
        _dataManager.clearAllSessions();
    }
    size_t SessionManager::GetSessionCount() const
    {
        std::lock_guard(_mutex);

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


