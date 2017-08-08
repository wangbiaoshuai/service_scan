#include "parse_policy.h"

#include <gtest/gtest.h>
#include "log.h"
#include <map>

using namespace cems::service::scan;
using namespace std;

#define LOG_CONFIG_PATH "../config/log4cplus.properties"

class FastScanTest : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        //fastscan_ = new FastScan();
    }
    
    static void TearDownTestCase()
    {
      /*  if(fastscan_ != NULL)
        {
            delete fastscan_;
            fastscan_ = NULL;
        }*/
    }

    //static FastScan* fastscan_;
};

TEST_F(FastScanTest, test_ReadPolicy_1)
{
    ParsePolicy::GetInstance().Init("policy1.xml");
    PolicyParam policy_param;
    int ret = ParsePolicy::GetInstance().ReadPolicy(policy_param);
    ASSERT_TRUE(ret == 0);

    ASSERT_TRUE(policy_param.interval_time == "5");
    ASSERT_TRUE(policy_param.update_time == 60 * 60 * 2);
    ASSERT_TRUE(policy_param.ip_range.size() == 2);
    map<string, string>::iterator it;
    map<string, string>& ip_range = policy_param.ip_range;
    int i = 0;
    for(it = ip_range.begin(); it != ip_range.end(); ++it)
    {
        switch(i)
        {
            case 1:
                cout << "it->first:" << it->first<<endl;
                ASSERT_TRUE(it->second == "org2");
                ASSERT_TRUE(it->first == "192.168.88.1-192.168.88.128");
                break;
            case 0:
                ASSERT_TRUE(it->second == "org1");
                ASSERT_TRUE(it->first == "192.168.1.1-192.168.1.254");
                break;
            default:
                break;
        }
        i++;
    }
}

TEST_F(FastScanTest, test_ReadPolicy_2)
{
    ParsePolicy::GetInstance().Init("policy2.xml");
    PolicyParam policy_param;
    int ret = ParsePolicy::GetInstance().ReadPolicy(policy_param);
    ASSERT_TRUE(ret == 0);

    ASSERT_TRUE(policy_param.interval_time == "60");
    ASSERT_TRUE(policy_param.update_time == 60 * 60 * 5);
    ASSERT_TRUE(policy_param.ip_range.size() == 0);
}

TEST_F(FastScanTest, test_ReadPolicy_3)
{
    ParsePolicy::GetInstance().Init("policy3.xml");
    PolicyParam policy_param;
    int ret = ParsePolicy::GetInstance().ReadPolicy(policy_param);
    ASSERT_TRUE(ret == 0);

    ASSERT_TRUE(policy_param.interval_time == "60");
    ASSERT_TRUE(policy_param.update_time == DEFAULT_UPDATE_TIME);
    ASSERT_TRUE(policy_param.ip_range.size() == 0);
}

TEST_F(FastScanTest, test_ReadPolicy_false)
{
    ParsePolicy::GetInstance().Init("../policy.xml");
    PolicyParam policy_param;
    int ret = ParsePolicy::GetInstance().ReadPolicy(policy_param);
    ASSERT_TRUE(ret == -1);
}

int main(int argc, char** argv)
{
    INIT_LOG(LOG_CONFIG_PATH);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
