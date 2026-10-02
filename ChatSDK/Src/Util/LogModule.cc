#include "../../Include/Common.h"
#include "../../Include/Util/LogModule.h"
namespace LogModule
{
std::shared_ptr<spdlog::logger> SpdLogPack::_logger = nullptr;
std::mutex SpdLogPack::_mutex;

const size_t MAX_FILE_SIZE = 1024 ; 
const size_t MAX_FILE_NUMBER = 3 ;
const size_t THREAD_POOL_SIZE = 8192 ;
    void SpdLogPack::SpdLogInit(std::string logname,std::string filename,
         spdlog::level::level_enum loglevel, 
         int mode)
    {
        if(nullptr == _logger)
        {
            std::lock_guard lock(_mutex);
            if(nullptr == _logger)
            {
                spdlog::init_thread_pool(THREAD_POOL_SIZE, 1);
                if(mode == CONSOLE__MODE)
                    _logger = spdlog::stdout_color_mt(logname);
                else if(mode == FILE_MODE)
                    _logger = spdlog::basic_logger_mt(logname, filename);
                else if(mode == ROTATING_MODE)
                    _logger = spdlog::rotating_logger_mt(logname, filename, 
                            MAX_FILE_SIZE, MAX_FILE_NUMBER);
                else if(mode == ASYNC_MODE)//异步日志默认输出到文件中吧
                    _logger = spdlog::basic_logger_mt<spdlog::async_factory>(logname,filename);
                _logger->set_level(loglevel);
                _logger->set_pattern("[%H : %M : %S][%-7l]%v");
            }
        }
    }
    std::shared_ptr<spdlog::logger> SpdLogPack::GetSpdLog() 
    {
        return SpdLogPack::_logger ;
    }
}