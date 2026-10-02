#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
int main()
{
    auto SpdLog = spdlog::stdout_color_mt("stdout");
    SpdLog->set_level(spdlog::level::info);
    SpdLog->set_pattern("[%H:%M:%S][%-7l]:%v");
    SpdLog->debug("[Hello MyName is {}]","JsonCpp");
    SpdLog->info("Hello MyName is {}","JavaScript");
    SpdLog->error("Hello MyName is {}", "ClaudeCode");
    auto SpdLog_1 = spdlog::basic_logger_mt("file_logger","file.det");
    SpdLog_1->set_level(spdlog::level::info);
    SpdLog_1->set_pattern("[%H:%M:%S][-7l]:%v");
    SpdLog_1->debug("[Hello MyName is {}]","JsonCpp");
    SpdLog_1->info("Hello MyName is {}","JavaScript");
    SpdLog_1->error("Hello MyName is {}", "ClaudeCode");
    auto Splog = spdlog::rotating_logger_mt("FileLogger","Log_.dat",1024,3);
    Splog->set_level(spdlog::level::err);
    Splog->set_pattern("[%H,%M,%S][-7l][-n]:%v");
    for(int i = 0; i < 10000; i++)
        Splog->error("Hello MyName is {}", "ClaudeCode");
    auto asyncLog = spdlog::stdout_color_mt<spdlog::async_factory>("asyncLog");
    asyncLog->set_level(spdlog::level::err);
    asyncLog->set_pattern("[%H,%M,%S][-7l][-n]:%v");
    asyncLog->error("[ASYNCLOG []]Hello MyName is {}", "ClaudeCode");
    asyncLog->error("[ASYNCLOG []]Hello MyName is {}", "ClaudeCode");
    asyncLog->error("[ASYNCLOG []]Hello MyName is {}", "ClaudeCode");
    asyncLog->error("[ASYNCLOG []]Hello MyName is {}", "ClaudeCode");
    asyncLog->error("[ASYNCLOG []]Hello MyName is {}", "ClaudeCode");
    return 0;
}