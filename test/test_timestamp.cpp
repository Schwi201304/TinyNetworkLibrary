#include "base/Timestamp.hpp"

#include <gtest/gtest.h>

using namespace schwi;
using namespace std;

TEST(TimestampTest, Timestamp)
{
    Timestamp t1(0);
    Timestamp t2(0);
    Timestamp t3(1);

    EXPECT_EQ(t1, t2);
    EXPECT_LT(t1, t3);

    EXPECT_EQ(t1.toString(), "1970-01-01 08:00:00.000000");
}

TEST(TimestampTest, AddTime)
{
    Timestamp t1(0);
    Timestamp t2 = addTime(t1, 1);
    Timestamp t3 = addTime(t1, 2);

    EXPECT_LT(t1, t2);
    EXPECT_LT(t2, t3);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}