#include <jsoncpp/json/json.h>
#include <string>
#include <iostream>
int main()
{
    Json::Value _value ;
    _value["姓名"] = "张三" ;
    _value["年纪"] = "18" ;
    _value["身高"] = "199" ;
    _value["体重"] = "80" ;
    _value["成绩"].append(98);
    _value["成绩"].append(99);
    _value["成绩"].append(152);
    _value["成绩"].append(121);
    _value["成绩"].append(111);
    _value["成绩"].append(456);
    _value["成绩"].append(222);
    _value["成绩"].append(996);
    std::string ret = _value.toStyledString();
    std::cout<<ret <<std::endl;
    return 0;
}
