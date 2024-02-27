#include "log/Logger.hpp"
#include "log/LogStream.hpp"
#include "log/AsyncLogFile.hpp"

#include <iostream>

using namespace schwi;
using namespace std;

int main()
{

    static_assert(std::is_base_of<ILogStreamBase, LogConsole>::value, "LogConsole must be a subclass of ILogStreamBase");
    static_assert(std::is_base_of<ILogStreamBase, LogFile>::value, "LogFile must be a subclass of ILogStreamBase");

    auto consoleLog = make_shared<LogConsole>();
    auto fileLog = make_shared<LogFile>("test.log");
    auto logger = make_shared<Logger>(Logger::DEBUG, consoleLog, fileLog);

    string str = "trace";
    const char *cstr = "debug";
    LOGGER_TRACE(logger, "test {} {}", str, 10);
    LOGGER_DEBUG(logger, "test {} {}", cstr, 1.0f);

    auto asyncFileLog = make_shared<AsyncLogFile>("test_async", 20 * 1024);
    asyncFileLog->start();
    auto logger2 = make_shared<Logger>(Logger::DEBUG, fileLog, asyncFileLog);
    for (int i; i < 1000; ++i)
    {
        if (i % 200 == 0)
        {
            cout << "sleep 1s" << endl;
            sleep(1);
        }
        LOGGER_DEBUG(logger2, "test {}", i);
    }
    return 0;
}