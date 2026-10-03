#include <string>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
namespace DaemonModule{
const std::string _DEV = "/dev/null" ;
    void EnableDaemon(int isnochdir , int isnoclose){
        signal(SIGCLD , SIG_IGN);
        signal(SIGPIPE , SIG_IGN);
        int pid = fork();
        if(pid > 0 ) exit(0);
        setsid();
        if(isnochdir == 0) chdir("/");
        if(isnoclose == 0){
            int fd = ::open(_DEV.c_str() , O_RDWR);
            if(fd < 0)
                std::cerr<<"Open The /dev/null Fail"<<std::endl;
            else {
                for(int i = 0 ;i < 3 ;i ++)
                    dup2(fd,i);
                close(fd);
            }
        }
    }
}