#include "log.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>

#include <QApplication>

namespace CorpusAnalyser
{

namespace
{

DateAndTime NowDateAndTime()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto fractional_seconds = ms.count() % 1000;
    std::stringstream ss;
#pragma warning (disable : 4996)
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X.") << fractional_seconds;
#pragma warning (default : 4996)
    return DateAndTime(ss.str());
}

}  // namespace

void Log(const LogMessage& msg)
{
    try
    {
        static std::ofstream log(QApplication::applicationDirPath().toStdString() + "/log.txt");
        static std::mutex mutex;

        {
            std::lock_guard<std::mutex> loch(mutex);
            const auto now = NowDateAndTime();
            log << std::this_thread::get_id() << " - " << now << ": " << msg << std::endl;
        }
    }
    catch(...)
    {
        exit(EXIT_FAILURE);
    }
}

}  // namespace CorpusAnalyser

