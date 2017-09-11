//当服务运行起来之后，用来测试服务是否已经注册到服务器上
#include <gtest/gtest.h>
#include "ConfigService.h"
#include "config_types.h"
#include <vector>
#include <string>

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TBinaryProtocol.h>
using namespace ::apache::thrift;
using namespace boost;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

using namespace std;
using namespace com::vrv::im::service;

typedef map<string, string> MAP_STRING;

#define SERVER_IP "192.168.133.73"
#define SERVER_PORT 8100

int QueryService(std::vector<ServiceConfigBean>& service_info, const std::string& service_code, const std::string& version)
{
    int ret = 0;
    boost::shared_ptr<TSocket> socket(new TSocket(SERVER_IP, SERVER_PORT));
    socket->setConnTimeout(1000 * 5);
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ConfigServiceClient client(protocol);
    try
    {
        transport->open();
        client.queryService(service_info, service_code, version);
        transport->close();
    }
    catch(TException& tx)
    {
        cout<<"TException:"<<tx.what()<<endl;
        ret = -1;
    }
    return ret;
}
#if 0
TEST(test_service_reg, queryservice)
{
    vector<ServiceConfigBean> service_info;
    std::string version = "1.0";
    string service_code = "00FF1100";
    ASSERT_TRUE(QueryService(service_info, service_code, version) == 0);
    ASSERT_TRUE(service_info.size() == 1);
    cout<<"service_info.size:"<<service_info.size()<<", name:"<<service_info[0].name<<endl;
    ASSERT_TRUE(service_info[0].name == "CEMS-SERVICE-SCAN");
    return;
}
#endif

int CompareMap(const MAP_STRING& map1, const MAP_STRING& map2)
{
    if(map1.size() != map2.size())
    {
        return -1;
    }

    MAP_STRING::const_iterator it1 = map1.begin();
    MAP_STRING::const_iterator it2 = map2.begin();
    do
    {
        cout<<"it1->first:"<<it1->first<<", it1->second:"<<it1->second<<endl;
        cout<<"it2->first:"<<it2->first<<", it2->second:"<<it2->second<<endl;
        if(it1->first.compare(it2->first) !=0 || it1->second.compare(it2->second) != 0)
        {
            return -1;
        }
    }while(++it1 != map1.end() && ++it2 != map2.end());

    return 0;
}

TEST(test_service_reg, comparemap)
{
    MAP_STRING map1, map2;
    map1["192.168.88.1-192.168.88.128"] = "org2";
    map2["192.168.88.1-192.168.88.128"] = "org2";

    map1["192.168.88.129-192.168.88.255"] = "org1";
    map2["192.168.88.129-192.168.88.255"] = "org1";
    ASSERT_TRUE(CompareMap(map1, map2) == 0);   
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
