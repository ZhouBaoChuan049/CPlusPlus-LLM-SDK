#include <sqlite3.h>
#include <string>
#include <iostream>

struct Student
{
    std::string _name;
    std::string _gender;
    int _age;
    double _GPA;

    Student(
        const std::string& name,
        const std::string& gender,
        int age,
        double GPA)
       :_name(name),
        _gender(gender),
        _age(age),
        _GPA(GPA){}
};

class Studentdb
{
public:
    Studentdb(const std::string& dbname)
    {
        int rc = sqlite3_open(dbname.c_str(),&_db);
        if(rc != SQLITE_OK)
        {
            std::cerr<<"打开数据库失败!"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_close(_db);
        }
        bool ret = CreateNewTable();
        if(ret == false)
            sqlite3_close(_db);
    }
    ~Studentdb(){
        if(_db != nullptr)
            sqlite3_close(_db);
    }
    //################增删改查####################
    bool Insert(const Student& student)
    {
        std::string InsertSQL = R"(
            INSERT INTO Student(name,gender,age,GPA)
            VALUES(?,?,?,?);
        )";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(
            _db,
            InsertSQL.c_str(),
            -1,
            &stmt,
            nullptr
        );
        if(rc != SQLITE_OK)
        {
            std::cerr<<"准备语句失败!"<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_bind_text(stmt,1,student._name.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,2,student._gender.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt,3,student._age);
        sqlite3_bind_double(stmt,4,student._GPA);
        // exec（文本执行器）：是一个从头到尾包办一切的快捷封装，负责将原始 
        //SQL 文本从解析、编译到执行一次性搞定（适合无参数的简单命令）。
        // step（字节码驱动器）：是一个只管向前推进的单步引擎，
        //负责推动已经编译好且绑定了数据的字节码（stmt）去执行一步（适合有参数或需要逐行处理结果的操作）。
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE)
        {
            std::cerr<<"执行语句失败!"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    }
    bool QuerryStudent(const std::string& name)
    {
        std::string QuerrySQL = R"(
            SELECT stuid,name,gender,age,GPA FROM Student WHERE name = ? ;
        )";
        sqlite3_stmt* stmt ;
        int rc = sqlite3_prepare(
            _db,
            QuerrySQL.c_str(),
            -1,
            &stmt,
            nullptr
        );
        if(rc != SQLITE_OK)
        {
            std::cerr<<"准备SQL语句失败"<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_bind_text(stmt , 1 , name.c_str(), -1 , SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if(rc != SQLITE_ROW && rc != SQLITE_DONE)
        {
            std::cerr<<"查找数据库失败"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
//        sqlite3_stmt的内部结构图:
//                     ┌──────────────────────────────────────────────┐
//                     │              sqlite3_stmt 句柄               │
//                     ├───────────────────────────────────────────── ┤
//   编译出来的指令 ──►  │ 1. 编译好的 VDBE 字节码 (指令)                  │
//                     │                                              │
//   当前行的内部缓存 ─►  │ 2. 数据结果行缓冲区 (Data Row Buffer)          │
//                     │   ┌──────┬───────┬────────┬─────┬─────────┐  │
//                     │   │  0   │   1   │   2    │  3  │    4    │  │
//                     │   ├──────┼───────┼────────┼─────┼─────────┤  │
//                     │   │ 1001 │ "张三" │  "男"  │ 20  │   3.8   │  │
//                     │   └──────┴───────┴────────┴─────┴─────────┘  │
//                     │                                              │
//   记录走到第几行 ──►   │ 3. 结果集游标 (Cursor)                        │
//                     └──────────────────────────────────────────────┘
        //现在我们要按照列号提取出我们想要的数据。
        int stuid = sqlite3_column_int(stmt,0);
        std::string QueryName = 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
        std::string Gender = 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
        int age = sqlite3_column_int(stmt,3);
        double gpa = sqlite3_column_double(stmt,4);

        //打印结果
        std::cout<<"查询到学生的信息:"<<std::endl;
        std::cout<<"stuid:"<<stuid<<std::endl;
        std::cout<<"name: "<<QueryName<<std::endl;
        std::cout<<"gender: "<<Gender<<std::endl;
        std::cout<<"age: "<<age<<std::endl;
        std::cout<<"gpa: "<<gpa<<std::endl;

        sqlite3_finalize(stmt);
        return true ;
    }

    bool QueryAllStudent()
    {
        std::string QuerySQL = R"(
            SELECT * FROM Student ;
        )";
        sqlite3_stmt* stmt ;
        int rc = sqlite3_prepare_v2(_db , QuerySQL.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            std::cerr<<"SQL语句准备失败!"<<std::endl;
            return false ;
        }
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_ROW && rc != SQLITE_DONE)
        {
            std::cerr<<"执行语句失败!"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        std::cout<<"------------ALL Student Informations-------------"<<std::endl;
        while(rc == SQLITE_ROW)
        {
            int stuid = sqlite3_column_int(stmt, 0);
            std::string queryName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string queryGender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            int queryAge = sqlite3_column_int(stmt, 3);
            double queryGap = sqlite3_column_double(stmt, 4);
            std::cout<<"查询到学生信息："<<std::endl;
            std::cout<<"stuid: "<<stuid<<" "
            <<"name: "<<queryName<<" "
            <<"gender: "<<queryGender<<" "
            <<"age: "<<queryAge<<" "
            <<"gap: "<<queryGap<<std::endl;
            
            rc = sqlite3_step(stmt);
        }
        if(rc != SQLITE_DONE)
        {
            std::cerr<<"提取结果失败:"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    }
    bool UpdateStudent(const std::string& name , const Student& info)
    {
        std::string updateSQL=R"(
            UPDATE Student SET gender = ?, age = ?, GPA = ? WHERE name = ? ;
        )";

        sqlite3_stmt* stmt ;
        int rc = sqlite3_prepare_v2(
            _db,
            updateSQL.c_str(),
            -1,
            &stmt,
            nullptr
        );
        if(rc != SQLITE_OK)
        {
            std::cerr<<"准备语句失败"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_bind_text(stmt, 1, info._gender.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, info._age);
        sqlite3_bind_double(stmt, 3, info._GPA);
        sqlite3_bind_text(stmt, 4, name.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE && rc != SQLITE_ROW){
            std::cerr<<"执行语句失败："<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        return true;
    }
    bool DeleteStudent(const std::string& name )
    {
        std::string DeleteSQL = R"(
            DELETE FROM Student WHERE name = ? ;
        )";
        sqlite3_stmt* stmt ;
        int rc = sqlite3_prepare(_db,DeleteSQL.c_str(),-1,&stmt,nullptr);
        if(rc != SQLITE_OK)
        {
            std::cerr<<"SQL语句准备失败!"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        rc = sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_TRANSIENT);
        if(rc !=SQLITE_OK)
        {
            std::cerr<<"SQL语句绑定失败!"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false ;
        }
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
            std::cerr<<"SQL语句执行失败!"<<sqlite3_errmsg(_db)<<std::endl;
            sqlite3_finalize(stmt);
            return false ;
        }
        sqlite3_finalize(stmt);
        return true ;
    }
    //############################################
private:
    bool CreateNewTable()
    {
        const std::string CreateTableSql = R"(
            CREATE TABLE IF NOT EXISTS Student(
                stuid INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT,
                gender TEXT,
                age INTEGER,
                GPA REAL
            )
        )";
        int rc = sqlite3_exec(
            _db,
            CreateTableSql.c_str(),
            nullptr,
            nullptr,
            nullptr
        );
        if(rc != SQLITE_OK){
            std::cerr<<"创建表失败:"<<sqlite3_errmsg(_db)<<std::endl;
            return false;
        }
        return true;
    }
    sqlite3* _db = nullptr;
};

int main()
{
    Student info1 = {"张三", "男", 18, 3.5};
    Student info2 = {"李四", "女", 19, 3.8};
    Student info3 = {"王五", "男", 20, 4.0};
    Student info4 = {"赵六", "女", 21, 4.2};

    Studentdb db("studentDB.db");
    db.Insert(info1);
    db.Insert(info2);
    db.Insert(info3);
    db.Insert(info4);

    // 查询所有学生信息
    db.QueryAllStudent();

    info3._GPA = 4.5;
    db.UpdateStudent(info3._name, info3);
    db.QuerryStudent(info3._name);

    // 删除学生信息
    db.DeleteStudent(info4._name);
    db.QueryAllStudent();

    return 0;
    return 0;
}