#include "parse_configure.h"

#include <gtest/gtest.h>
#include <string>
#include <iostream>

using namespace std;
using namespace cems::service::scan;

TEST(test_configure, test_get_property)
{
    ParseConfigure::GetInstance().Init();
    string key="service.ip";
    string value;
    ASSERT_TRUE(ParseConfigure::GetInstance().GetProperty(key, value));
    ASSERT_TRUE(value == "192.168.133.93");

    key = "updateUrl";
    ASSERT_TRUE(ParseConfigure::GetInstance().GetProperty(key, value));
    ASSERT_TRUE(value == "http://192.168.0.24:8080/CEMS/serverServlet");

    key = "nothing";
    ASSERT_FALSE(ParseConfigure::GetInstance().GetProperty(key, value));
}

TEST(test_configure, test_set_property)
{
    ParseConfigure::GetInstance().Init("./config.properties");
    string key = "sleep";
    string value = "10";
    ParseConfigure::GetInstance().SetProperty(key, value);

    string res;
    ASSERT_TRUE(ParseConfigure::GetInstance().GetProperty(key, res));
    ASSERT_TRUE(res == "10");

    key = "installTime";
    value = "2017/7/17";
    ParseConfigure::GetInstance().SetProperty(key, value);
    
    ASSERT_TRUE(ParseConfigure::GetInstance().GetProperty(key, res));
    ASSERT_TRUE(res == "2017/7/17");
}

TEST(test_configure, test_get_empty_property)
{
    ParseConfigure::GetInstance().Init("./config.properties");
    string key = "service.installTime";
    string value;
    ASSERT_TRUE(ParseConfigure::GetInstance().GetProperty(key, value));
    ASSERT_TRUE(value.empty());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
