#include "base/ThreadPool.hpp"
#include <iostream>
#include <vector>
#include <future>

#include <gtest/gtest.h>

using namespace std;
using namespace schwi;

void task(int a, int b, std::promise<int> &output)
{
    output.set_value(a + b);
}

// 测试ThreadPool是否能正确执行任务
TEST(ThreadPoolTest, AddTask)
{
    ThreadPool pool(4);
    pool.start();

    std::vector<int> a{1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int> b{8, 7, 6, 5, 4, 3, 2, 1};

    for (int i = 0; i < 8; ++i)
    {
        std::promise<int> prom;
        auto fut = prom.get_future();
        pool.addTask([&]()
                     { task(a[i], b[i], prom); });
        EXPECT_EQ(fut.get(), a[i] + b[i]);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}