#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
int main()
{
    std::string JsonString = 
        R"({"体重":"80","姓名":"张三","性别":"男性","成绩":[98,99,152,121,111],"身高":"199"})";
    Json::CharReaderBuilder builder ;
    
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json:: Value values ;
    std::string res ;
    bool check = reader->parse(JsonString.c_str(), 
                  JsonString.c_str() + JsonString.size(),
                  &values , 
                  &res);
    if(!check)
    {
        std::cout<<"JsonReader Fail! res:["<<res<<"]"<<std::endl;
        return -1;
    }
    if(values["体重"].isString())
        std::cout<<R"(values["体重"] = )"<<values["体重"]<<std::endl;
    if(values["成绩"].isArray())
        std::cout<<R"(values["成绩"] = )"<<values["成绩"]<<std::endl;
    return 0 ;
}