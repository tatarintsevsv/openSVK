#include "../config/config.h"
#include <gtest/gtest.h>
#include <string>
#include <iostream>
 
TEST(configTest, Test_Config_Open) {
    ASSERT_TRUE(configInit("../../../bin/config.xml","TEST"));
};

TEST(configTest, Test_Config_Reading) {
    ASSERT_EQ(20, configReadInt("//config/global/@pollinterval",0));
    ASSERT_STREQ("telnet", configReadString("//config/stage[1]/@type",""));
    ASSERT_EQ(6, atoi(configGetNodeSet("count(//config/stage)")));

}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
