#include "log/AsyncLogFile.hpp"
#include "log/Logger.hpp"
#include "log/LogStream.hpp"
#include <iostream>
#include <chrono>

using namespace schwi;
using namespace std;

int main()
{
    const int num_logs = 1000000;

    auto asyncLog = make_shared<AsyncLogFile>("asynclog", 1000);
    auto logger = make_shared<Logger>(Logger::DEBUG, std::vector<Logger::LogStreamPtr>{asyncLog});

    asyncLog->start();

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_logs; ++i)
    {
        LOGGER_DEBUG(logger, "Hello world from log {}", i);
    }

    auto end = std::chrono::high_resolution_clock::now();

    asyncLog->stop();

    std::chrono::duration<double> diff = end - start;
    std::cout << "Logged " << num_logs << " entries in " << diff.count() << " seconds.\n";
    std::cout << "That's " << num_logs / diff.count() << " logs per second.\n";

    return 0;
}