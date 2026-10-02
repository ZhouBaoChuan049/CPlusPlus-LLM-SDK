#include <iostream>
#include <string>
#include "../../Third_Party/Httplib/httplib.h"



void GetHome(httplib::Request req , httplib::Response resp){
    std::cout<<req.method<<std::endl;
    std::cout<<req.path<<std::endl;
    std::cout<<req.body<<std::endl;
    for(auto it : req.headers){
        std::cout<<it.first<<it.second<<std::endl;
    }
    return ;
}

int main()
{
    httplib::Server svr ;
    svr.Get("/home", GetHome);
    svr.Get(R"(/number(\d+))",[](httplib::Request req , httplib::Response resp){
            std::cout<<req.method<<std::endl;
            std::cout<<req.path<<std::endl;
            std::cout<<req.body<<std::endl;
            for(auto it : req.headers){
                std::cout<<it.first<<it.second<<std::endl;
            for(auto it : req.matches){
                std::cout<<it<<std::endl;
            }
    }
    });
    svr.listen("0.0.0.0" , 9090);
    return 0;
}