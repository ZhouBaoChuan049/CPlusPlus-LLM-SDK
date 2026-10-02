#include "../Include/Common.h"
#include "../Include/Util/LogModule.h"
#include "../Include/CommonStruct.h"
#include "../Include/DataManager.h"
namespace Cplusplus_LLM_Provider
{
    bool DataManager::InitDataBase()
    {
        _mutex.lock();
        std::string CreateSessionTableSQL = R"(
            CREATE TABLE IF NOT EXISTS Sessions
            (
                _SessionID TEXT PRIMARY KEY ,
                _ModelNameUsed TEXT,
                _UserName TEXT,
                _TimeCreate INTEGER,
                _LastTime INTEGER
            )
        )";
        int ret = sqlite3_exec(_db, CreateSessionTableSQL.c_str(), nullptr, nullptr, nullptr);
        if(ret !=SQLITE_OK)
        {
            _mutex.unlock();
            LogModule::CRITICAL("创建Session表失败,失败原因{}",sqlite3_errmsg(_db));
            return false ;
        }
        LogModule::INFO("创建Session表成功");

        std::string CreateMessageTableSQL = R"(
            CREATE TABLE IF NOT EXISTS Messages
            (
                _MessageID TEXT PRIMARY KEY,
                _SessionID TEXT,
                _Role TEXT,
                _Content TEXT,
                _Time INTEGER
            )
        )";
        ret = sqlite3_exec(_db, CreateMessageTableSQL.c_str(), nullptr, nullptr, nullptr);
        if(ret !=SQLITE_OK)
        {
            _mutex.unlock();
            LogModule::CRITICAL("创建Message表失败,失败原因{}",sqlite3_errmsg(_db));
            return false ;
        }
        LogModule::INFO("创建Message表成功");

        std::string CreateUserTableSQL = R"(
            CREATE TABLE IF NOT EXISTS Users
            (
                _UserName TEXT PRIMARY KEY ,
                _PasswordHash TEXT,
                _Salt TEXT
            )
        )";
        ret = sqlite3_exec(_db, CreateUserTableSQL.c_str(), nullptr, nullptr, nullptr);
        if(ret !=SQLITE_OK)
        {
            _mutex.unlock();
            LogModule::CRITICAL("创建User表失败,失败原因{}",sqlite3_errmsg(_db));
            return false ;
        }
        LogModule::INFO("创建User表成功");
        _mutex.unlock();
        return true ;
    }
    DataManager::DataManager(std::string dbname)
    {
        int ret = sqlite3_open(dbname.c_str(),&_db);
        if(ret != SQLITE_OK)
        {
            LogModule::CRITICAL("打开数据库{}失败,失败原因{}",dbname,sqlite3_errmsg(_db));
            std::string Errmsg = "打开数据库失败";
            throw Errmsg ;
        }
        LogModule::INFO("打开数据库成功");
        bool check = InitDataBase();
        if(check == false)
        {
            std::string Errmsg = "初始化数据库失败";
            throw Errmsg ;
        }
    }
    DataManager::~DataManager()
    {
        if(_db != nullptr)
        {
            sqlite3_close(_db);
            _db = nullptr;
        }
    }
    bool DataManager::InsertSession(const Session& session)
    {
        _mutex.lock();
        std::string InsertSQL = R"(
            INSERT INTO Sessions (_SessionID,_ModelNameUsed,_UserName,_TimeCreate,_LastTime)
            VALUES(?,?,?,?,?);
        )";
        sqlite3_stmt* stmt ;

        int ret = sqlite3_prepare(_db,InsertSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("InsertSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        sqlite3_bind_text(stmt ,1,session._SessionID.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt ,2,session._ModelNameUsed.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt ,3,session._UserName.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt ,4,session._TimeCreate);
        sqlite3_bind_int(stmt ,5,session._LastTime);

        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            LogModule::ERROR("InsertSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        LogModule::INFO("插入Session{}成功!", session._SessionID);
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return true;
    }
    std::shared_ptr<Session> DataManager::GetSession(const std::string& sessionId)const
    {
        _mutex.lock();
        std::string GetSQL = R"( 
            SELECT _UserName, _ModelNameUsed, _TimeCreate, _LastTime FROM Sessions WHERE _SessionID=?;
        )";
        sqlite3_stmt*stmt ;
        int ret = sqlite3_prepare(_db,GetSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("GetSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return nullptr ;
        }
        sqlite3_bind_text(stmt,1,sessionId.c_str(),-1,SQLITE_TRANSIENT);
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_ROW)
        {
            LogModule::ERROR("GetSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return nullptr ;
        }
        std::string UserName = 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
        std::string ModelName = 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
        time_t TimeCreate = sqlite3_column_int(stmt,2);
        time_t LastTime = sqlite3_column_int(stmt,3);
        Session session(ModelName, UserName);
        session._LastTime = LastTime;
        session._TimeCreate = TimeCreate;
        session._SessionID = sessionId;
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return std::make_shared<Session>(session);
    }
    bool DataManager::updateSessionTimestamp(const std::string& sessionId, std::time_t timestamp)
    {
        _mutex.lock();
        std::string updateSQL = R"(
            UPDATE Sessions SET _LastTime = ? WHERE _SessionID=?;
        )";  
        sqlite3_stmt* stmt ;
        int ret = sqlite3_prepare(_db,updateSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("updateSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false;
        }
        sqlite3_bind_int(stmt,1,timestamp);
        sqlite3_bind_text(stmt,2,sessionId.c_str(),-1,SQLITE_TRANSIENT);
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            LogModule::ERROR("updateSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        sqlite3_finalize(stmt);
        LogModule::INFO("updateSQL执行成功!");
        _mutex.unlock();
        return true ;
    }
    bool DataManager::deleteSession(const std::string& sessionId)
    {
        _mutex.lock();
        std::string DelSQL = R"(
            DELETE FROM Sessions WHERE _SessionID=?;
        )";
        sqlite3_stmt* stmt ;
        int ret = sqlite3_prepare(_db,DelSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("DelSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false;
        }
        sqlite3_bind_text(stmt,1,sessionId.c_str(),-1,SQLITE_TRANSIENT);
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            LogModule::ERROR("DelSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        sqlite3_finalize(stmt);
        LogModule::INFO("DelSQL执行成功!");
        _mutex.unlock();
        return true ;
    }
    std::vector<std::string> DataManager::getAllSessionIds()const
    {
        _mutex.lock();
        std::string selectALLSQL = R"(
            SELECT _SessionID FROM Sessions ORDER BY _LastTime DESC ;
        )";
        sqlite3_stmt*stmt ;
        int ret = sqlite3_prepare(_db,selectALLSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("selectALLSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return {} ;
        }
        std::vector<std::string> IDcollections ;
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string _id = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
            IDcollections.push_back(_id);
        }
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return IDcollections;
    }
    std::vector<std::shared_ptr<Session>> DataManager::getAllSessions()const
    {
        _mutex.lock();
        std::string selectALLSQL = R"(
            SELECT _SessionID, _UserName, _ModelNameUsed, _TimeCreate, _LastTime FROM Sessions ORDER BY _LastTime DESC ;
        )";
        sqlite3_stmt*stmt ;
        int ret = sqlite3_prepare(_db,selectALLSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("selectALLSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return {} ;
        }
        std::vector<std::shared_ptr<Session>> Sessionscollections ;
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string id = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
            std::string userName = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
            std::string modelname = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
            time_t TimeCreate = sqlite3_column_int(stmt,3);
            time_t LastTime = sqlite3_column_int(stmt,4);

            std::shared_ptr<Session> _session = std::make_shared<Session>(modelname, userName);
            _session->_SessionID = id ;
            _session->_TimeCreate = TimeCreate;
            _session->_LastTime = LastTime;
            Sessionscollections.push_back(_session);
        }
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return Sessionscollections;
    }
    std::vector<std::shared_ptr<Session>> DataManager::getSessionsByUser(const std::string& username)const
    {
        _mutex.lock();
        std::string selectSQL = R"(
            SELECT _SessionID, _ModelNameUsed, _TimeCreate, _LastTime FROM Sessions WHERE _UserName=? ORDER BY _LastTime DESC ;
        )";
        sqlite3_stmt*stmt ;
        int ret = sqlite3_prepare(_db,selectSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("selectSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return {} ;
        }
        sqlite3_bind_text(stmt,1,username.c_str(),-1,SQLITE_TRANSIENT);
        std::vector<std::shared_ptr<Session>> Sessionscollections ;
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string id = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
            std::string modelname = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
            time_t TimeCreate = sqlite3_column_int(stmt,2);
            time_t LastTime = sqlite3_column_int(stmt,3);

            std::shared_ptr<Session> _session = std::make_shared<Session>(modelname, username);
            _session->_SessionID = id ;
            _session->_TimeCreate = TimeCreate;
            _session->_LastTime = LastTime;
            Sessionscollections.push_back(_session);
        }
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return Sessionscollections;
    }
    bool DataManager::clearAllSessions()
    {
        _mutex.lock();
        std::string clearAllSQL = R"(
            DELETE FROM Sessions ;
        )";
        sqlite3_stmt* stmt ;
        int ret = sqlite3_prepare(_db,clearAllSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("clearAllSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return 0;
        }
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            LogModule::ERROR("clearAllSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return 0;
        }
        sqlite3_finalize(stmt);
        LogModule::INFO("clearAllSQL执行成功!");
        _mutex.unlock();
        return true;
    }
    size_t DataManager::getSessionCount()const
    {
        _mutex.lock();
        std::string CountSQL = R"(
            SELECT COUNT(*) FROM Sessions ;
        )";
        sqlite3_stmt* stmt ;
        int ret = sqlite3_prepare(_db,CountSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("CountSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return 0;
        }
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_ROW)
        {
            LogModule::ERROR("CountSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return 0;
        }
        int count = sqlite3_column_int(stmt,0);
        sqlite3_finalize(stmt);
        LogModule::INFO("CountSQL执行成功!");
        _mutex.unlock();
        return count;
    }
    /////////////////////////////////////////////////////////////////////////////////////
    bool DataManager::insertMessage(const std::string& sessionId, const Message& message)
    {
        _mutex.lock();
        std::string InsertSQL = R"(
            INSERT INTO Messages (_MessageID,_SessionID,_Role,_Content,_Time)
            VALUES(?,?,?,?,?);
        )";
        sqlite3_stmt* stmt ;

        int ret = sqlite3_prepare(_db,InsertSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("InsertSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        sqlite3_bind_text(stmt ,1,message._MessageID.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt ,2,sessionId.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt ,3,message._Role.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt ,4,message._Content.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt ,5,message._Time);

        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            LogModule::ERROR("InsertSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        LogModule::INFO("插入Messages{}成功!", message._MessageID);
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return true;
    }
    std::vector<Message> DataManager::getSessionMessages(const std::string& sessionId)const
    {
        _mutex.lock();
        std::string GetSQL = R"( 
            SELECT _MessageID, _Role, _Content, _Time FROM Messages WHERE _SessionID=?;
        )";
        sqlite3_stmt*stmt ;
        int ret = sqlite3_prepare(_db,GetSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("GetSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return {} ;
        }
        std::vector<Message> messages ;
        sqlite3_bind_text(stmt,1,sessionId.c_str(),-1,SQLITE_TRANSIENT);
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string MessageID = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
            std::string Role = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
            std::string Content = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
            time_t Time = sqlite3_column_int(stmt,3);
            Message _message(Role,Content) ;
            _message._MessageID = MessageID ;
            _message._SessionID = sessionId ;
            _message._Time = Time ;
            messages.push_back(_message);
        }
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return messages;
    }
    bool DataManager::deleteSessionMessages(const std::string& sessionId)
    {
        _mutex.lock();
        std::string DelSQL = R"(
            DELETE FROM Messages WHERE _SessionID=?;
        )";
        sqlite3_stmt* stmt ;
        int ret = sqlite3_prepare(_db,DelSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("DelSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false;
        }
        sqlite3_bind_text(stmt,1,sessionId.c_str(),-1,SQLITE_TRANSIENT);
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            LogModule::ERROR("DelSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        sqlite3_finalize(stmt);
        LogModule::INFO("DelSQL执行成功!");
        _mutex.unlock();
        return true ;
    }
    /////////////////////////////////////////////////////////////////////////////////////
    bool DataManager::insertUser(const std::string& username, const std::string& passwordHash, const std::string& salt)
    {
        _mutex.lock();
        std::string InsertSQL = R"(
            INSERT INTO Users (_UserName,_PasswordHash,_Salt)
            VALUES(?,?,?);
        )";
        sqlite3_stmt* stmt ;
        int ret = sqlite3_prepare(_db,InsertSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("InsertUserSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        sqlite3_bind_text(stmt ,1,username.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt ,2,passwordHash.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt ,3,salt.c_str(),-1,SQLITE_TRANSIENT);
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_DONE)
        {
            LogModule::ERROR("InsertUserSQL执行失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return false ;
        }
        LogModule::INFO("插入用户{}成功!", username);
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return true;
    }
    std::shared_ptr<User> DataManager::getUser(const std::string& username)const
    {
        _mutex.lock();
        std::string GetSQL = R"( 
            SELECT _UserName,_PasswordHash,_Salt FROM Users WHERE _UserName=?;
        )";
        sqlite3_stmt* stmt ;
        int ret = sqlite3_prepare(_db,GetSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("GetUserSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return nullptr ;
        }
        sqlite3_bind_text(stmt,1,username.c_str(),-1,SQLITE_TRANSIENT);
        ret = sqlite3_step(stmt);
        if(ret != SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return nullptr ;
        }
        std::string name = 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
        std::string passwordHash = 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
        std::string salt = 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return std::make_shared<User>(name,passwordHash,salt);
    }
    std::vector<std::shared_ptr<User>> DataManager::getAllUsers()const
    {
        _mutex.lock();
        std::string selectALLSQL = R"(
            SELECT _UserName,_PasswordHash,_Salt FROM Users ;
        )";
        sqlite3_stmt* stmt ;
        int ret = sqlite3_prepare(_db,selectALLSQL.c_str(),-1,&stmt,nullptr);
        if(ret != SQLITE_OK)
        {
            LogModule::ERROR("getAllUsersSQL准备失败,失败原因{}",sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            _mutex.unlock();
            return {} ;
        }
        std::vector<std::shared_ptr<User>> userCollection ;
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string username = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
            std::string passwordHash = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
            std::string salt = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
            userCollection.push_back(std::make_shared<User>(username,passwordHash,salt));
        }
        sqlite3_finalize(stmt);
        _mutex.unlock();
        return userCollection;
    }
}
