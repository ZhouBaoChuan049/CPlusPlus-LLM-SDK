#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
int main()
{
    Json::Value values ;
    values["姓名"] = "张三";
    values["性别"] = "男性";
    values["身高"] = "199" ;
    values["体重"] = "80" ;
    values["成绩"].append(98);
    values["成绩"].append(99);
    values["成绩"].append(152);
    values["成绩"].append(121);
    values["成绩"].append(111);
    Json::StreamWriterBuilder builder ;
    builder["commentStyle"] = "None" ;
    builder["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> swp (builder.newStreamWriter());
    std::stringstream ss ;
    int check = swp->write(values , &ss);
    if(check != 0)
    {
        std::cout<<"Json write fail!"<<std::endl;
        return -1 ;
    }
    std::string ret = ss.str();
    std::cout<<ret<<std::endl;
    return 0 ;
}