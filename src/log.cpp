#include "webserver/log.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
namespace webserver
{
    void log(const std::string& msg)
    {
        auto now=std::chrono::system_clock::now();
        auto t=std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_r(&t,&tm);
        std::cout << "[" << std::put_time(&tm,"%Y-%m-%d %H:%M:%S") 
                  << "] [INFO] " << msg << "\n";
    }
}
