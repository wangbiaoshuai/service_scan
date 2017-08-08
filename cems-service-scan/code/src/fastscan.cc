#include "fastscan.h"

#include "log.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "parse_policy.h"
#include "parse_configure.h"
#include "DetectHost.h"
#include "service_reg.h"
#include "json/json.h"
#include "common_function.h"

namespace cems{ namespace service{ namespace scan{
using namespace std;

FastScan::FastScan():
scan_thread_(),
update_policy_thread_(),
stop_scan_(false),
stop_update_policy_(false)
{
}

#if 0
long long FastScan::SwitchTime(string cycle)
{
    if(cycle.empty())
        return 0;

    long long time = 1;
    size_t pos = cycle.find("*");
    while(pos != string::npos)
    {
        time *= atoll(cycle.substr(0, pos+1).c_str());
        cycle.erase(0, pos+1);
        pos = cycle.find("*");
    }
    if(!cycle.empty())
    {
        time *= atoll(cycle.c_str());
    }
    return time;
}
#endif

FastScan::~FastScan()
{
}

void* start_scan_function(void* context)
{
    FastScan* ctx = (FastScan*)context;
    ctx->StartScan();
    LOG_INFO("Scan thread stoped.");
    pthread_exit(NULL);
}

void* start_update_polity_function(void* context)
{
    FastScan* ctx = (FastScan*)context;
    ctx->StartUpdatePolicy();
    LOG_INFO("update policy thread stoped.");
    pthread_exit(NULL);
}

int FastScan::Start()
{
    int res = pthread_create(&scan_thread_, NULL, start_scan_function, this);
    if(res != 0)
    {
        LOG_ERROR("create scan thread failed: " << strerror(res));
        return -1;
    }

    res = pthread_create(&update_policy_thread_, NULL, start_update_polity_function, this);
    if(res != 0)
    {
        LOG_ERROR("create update policy thread failed: " << strerror(res));
        return -1;
    }
    return 0;
}

int FastScan::Stop()
{
    int ret = 0;
    stop_scan_ = true;
    int res = pthread_join(scan_thread_, NULL);
    if(res != 0)
    {
        ret = -1;
        LOG_ERROR("scan thread can not joined: " << strerror(res));
    }
    else
    {
        LOG_INFO("scan thread has been joined");
    }

    stop_update_policy_ = true;
    res = pthread_join(update_policy_thread_, NULL);
    if(res != 0)
    {
        ret = -1;
        LOG_ERROR("update policy thread can not joined: " << strerror(res));
    }
    else
    {
        LOG_INFO("update policy thread has been joined");
    }
    return ret;
}

int FastScan::CompareMap(const MAP_STRING& map1, const MAP_STRING& map2)
{
    if(map1.size() != map2.size())
    {
        return -1;
    }

    MAP_STRING::const_iterator it1 = map1.begin();
    MAP_STRING::const_iterator it2 = map2.begin();
    while(it1 != map1.end() && it2 != map2.end())
    {
        if(it1->first.compare(it2->first) !=0 || it1->second.compare(it2->second) != 0)
        {
            return -1;
        }
        ++it1; ++it2;
    }

    return 0;
}

int FastScan::ParseIpRange(const string& ip_range, MAP_STRING& range)
{
    if(ip_range.empty())
    {
        return -1;
    }
    range.clear();

    vector<string> tmp;
    char* p = strtok((char*)ip_range.c_str(), ";");
    while(p != NULL)
    {
        tmp.push_back(p);
        p = strtok(NULL, ";");
    }

    for(unsigned int i = 0; i < tmp.size(); i++)
    {
        std::string  szTemp1, szTemp2;
        szTemp1  =  szTemp2 = tmp[i];
        size_t index = szTemp1.find("-");
        if(index != std::string::npos)
        {
            szTemp1.erase(index);
            szTemp2.erase(0, index + 1);

            range[szTemp1] = szTemp2;
        }
    }

    return (int)range.size();
}


int FastScan::StartScan()
{
    map<string, string> old_ip_range;
    if(DetectInit() != 1)
    {
        LOG_ERROR("StartScan: DetectInit failed");
        return -1;
    }
    ParsePolicy::GetInstance().Init();
    while(!stop_scan_)
    {
        LOG_INFO("StartScan: begin scan...");
        PolicyParam policy_param;
        if(ParsePolicy::GetInstance().ReadPolicy(policy_param) == -1 || policy_param.ip_range.empty())
        {
            LOG_WARN("ip range is empty, sleep 60S...");
            sleep(60);
            continue;
        }

        string key, area_id;
        key = "service.serverAreaId";
        if(ParseConfigure::GetInstance().GetProperty(key, area_id) == false || area_id.empty())
        {
            area_id = DEFAULT_AREA_ID;
        }

        if(CompareMap(old_ip_range, policy_param.ip_range) != 0)
        {
            LOG_INFO("StartScan: ip range has changed.");
            DetectChange();
        }

        old_ip_range = policy_param.ip_range;

        map<string, string>::iterator it;
        for(it = old_ip_range.begin(); it != old_ip_range.end(); ++it)
        {
            map<string, string> range;
            if(ParseIpRange(it->first, range) <= 0)
            {
                LOG_WARN("StartScan: prase ip range is empty:" << it->first.c_str());
                continue;
            }
            if(DetectUnRegist(&range, area_id, it->second) != 0)
            {
                LOG_ERROR("StartScan: DetectUnRegist failed");
            }
        }

        if(DetectClose() != 0)
        {
            LOG_ERROR("StartScan: DetectClose failed");
        }

        int sleep_time = atoi(policy_param.interval_time.c_str()); 
        LOG_INFO("StartScan: scan end. sleep " << sleep_time << "S...");
        sleep(sleep_time);
    }
    return 0;
}

int FastScan::StartUpdatePolicy()
{
    string maxcode = SERVICE_CODE_ADDRESS;
    string mincode = MINCODE_ADDRESS_UPDATEPOLICY;
    ParsePolicy::GetInstance().Init();
    while(!stop_update_policy_)
    {
        PolicyParam policy_param;
        do
        {    
            if(ParsePolicy::GetInstance().ReadPolicy(policy_param) == -1 || policy_param.update_time == 0)
            {
                LOG_ERROR("StartUpdatePolicy: ReadPolicy failed.");
            }
            ServiceReg policy_fetch;
            string address_ip, address_port;
            string jresult, jdata;

            string crc;
            if(ParsePolicy::GetInstance().GetPolicyCrc(crc) == false)
            {
                LOG_ERROR("StartUpdatePolicy: GetPolicyCrc failed.");
                break;
            }
            Json::Value root;
            string serverId = GenericUUID(GetCurrentIp());
            root["serverId"] = serverId;//GenericUUID(GetCurrentIp());
            root["serviceCode"] = MAXCODE_SCAN_SERVICE;
            root["crc"] = crc;
            Json::FastWriter writer;
            jdata = writer.write(root);

            LOG_DEBUG("StartUpdatePolicy: begin get policy from address service, serverId="<<serverId<<", serviceCode="<<MAXCODE_SCAN_SERVICE<<", crc="<<crc);

            if(policy_fetch.Fetch(maxcode, "", address_ip, address_port) == false)
            {
                LOG_ERROR("StartUpdatePolicy: Fetch "<<maxcode<<" ip&port failed.");
                break;
            }
            jresult = policy_fetch.RequestService(address_ip, address_port, maxcode, mincode, false, jdata);
            if(jresult.empty())
            {
                LOG_ERROR("StartUpdatePolicy: request service failed.");
                break;
            }

            Json::Reader reader;
            Json::Value jread, jtmp, jpolicy;
            if(!reader.parse(jresult, jread))
            {
                LOG_ERROR("StartUpdatePolicy: parse jresult failed.");
                break;
            }

            if(jread["result"].compare("0") != 0)
            {
                LOG_ERROR("StartUpdatePolicy: result=0, update policy file illegal.");
                break;
            }
            jtmp = jread["jdata"];
            if(jtmp.size() > 0)
            {
                int i = 0;
                jpolicy = jtmp[i];
            }

            std::string is_change = jpolicy["isChange"].isNull() ?  "" : jpolicy["isChange"].asString();
            if(is_change == "0")
            {
                LOG_INFO("StartUpdatePolicy: policy not change.");
                break;
            }
            else
            {
                LOG_INFO("StartUpdatePolicy: policy has changed, Update policy file...");
                std::string xml = jpolicy["policyXml"].isNull() ?  "" : jpolicy["policyXml"].asString();
                if(ParsePolicy::GetInstance().WritePolicy(xml) <= 0)
                {
                    LOG_ERROR("StartUpdatePolicy: WritePolicy error.");
                }
                else
                {
                    LOG_INFO("StartUpdatePolicy: WritePolicy success.");
                }
            }
        }while(0);
        long long sleep_time = policy_param.update_time;
        if(sleep_time <= 0)
        {
            sleep_time = DEFAULT_UPDATE_TIME;
        }

        LOG_INFO("StartUpdatePolicy: update policy end. Sleep "<<sleep_time<<"S...");
        sleep(sleep_time);
    }

    return 0;
}
}}}
