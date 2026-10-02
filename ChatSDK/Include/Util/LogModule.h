#ifndef __LOG_MODULE__
#define __LOG_MODULE__
#include "../Common.h"

namespace LogModule
{
    class SpdLogPack
    {
    public:
        SpdLogPack() = delete ;
        SpdLogPack(SpdLogPack& spdlog) = delete ;
        static void SpdLogInit(std::string logname,std::string filename, 
            spdlog::level::level_enum loglevel = spdlog::level::info
            ,  int mode = FILE_MODE) ;
        static std::shared_ptr<spdlog::logger> GetSpdLog() ;
        
    private:
        static std::shared_ptr<spdlog::logger> _logger ; 
        static std::mutex _mutex;
    };
#define TRACE(format, ...) SpdLogPack::GetSpdLog()->trace(std::string("[{:>10}][{:<4}]")+format,__FILE__,__LINE__,##__VA_ARGS__);
#define DEBUG(format, ...) SpdLogPack::GetSpdLog()->debug(std::string("[{:>10}][{:<4}]")+format,__FILE__,__LINE__,##__VA_ARGS__);
#define INFO(format, ...) SpdLogPack::GetSpdLog()->info(std::string("[{:>10}][{:<4}]")+format,__FILE__,__LINE__,##__VA_ARGS__);
#define WARNING(format, ...) SpdLogPack::GetSpdLog()->warn(std::string("[{:>10}][{:<4}]")+format,__FILE__,__LINE__,##__VA_ARGS__);
#define ERROR(format, ...) SpdLogPack::GetSpdLog()->error(std::string("[{:>10}][{:<4}]")+format,__FILE__,__LINE__,##__VA_ARGS__);
#define CRITICAL(format, ...) SpdLogPack::GetSpdLog()->critical(std::string("[{:>10}][{:<4}]")+format,__FILE__,__LINE__,##__VA_ARGS__);
}
#endif