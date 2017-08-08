#include "parse_policy.h"

#include "log.h"
#include "tinyxml.h"
#include "common_function.h"

namespace cems{ namespace service{ namespace scan{
using namespace std;

ParsePolicy::ParsePolicy() :
policy_mutex_(),
policy_file_("")
{
}

ParsePolicy::~ParsePolicy()
{
}

void ParsePolicy::Init(const string& file_name)
{
    policy_file_ = file_name;
    return;
}

ParsePolicy& ParsePolicy::GetInstance()
{
    static ParsePolicy parse_policy;
    return parse_policy;
}

long long ParsePolicy::SwitchTime(string cycle)
{
    if(cycle.empty())
        return 0;

    long long time = 1;
    int i = 1;
    size_t pos = cycle.find(" ");
    while(pos != string::npos)
    {
        string time_circle = cycle.substr(0, pos+1);
        size_t time_pos = time_circle.find("/");
        if(time_pos != string::npos)
        {
            time_circle.erase(0, time_pos+1);
            switch(i)
            {
                case 1:
                    time *= atoi(time_circle.c_str());
                    break;
                case 2:
                    time *= 60 * atoi(time_circle.c_str());
                    break;
                case 3:
                    time *= 60 * 60 * atoi(time_circle.c_str());
                    break;
                case 4:
                    time *= 60 * 60 * 24 * atoi(time_circle.c_str());
                    break;
                case 5:
                    time *= 60 * 60 * 24 * 30 * atoi(time_circle.c_str());
                    break;
                default:
                    break;
            }

            return time;
        }
        cycle.erase(0, pos+1);
        pos = cycle.find(" ");
        i++;
    }
    if(!cycle.empty() && i == 5)
    {
        pos = cycle.find("/");
        if(pos != string::npos)
        {
            time *= 60 * 60 * 24 * 30;
        }
    }
    return time;
}

int ParsePolicy::ReadPolicy(PolicyParam& policy_param)
{
    TiXmlDocument *document = NULL;
    TiXmlElement  *root_element = NULL;
    TiXmlElement  *age_element  = NULL;
    TiXmlElement  *child_element  = NULL;
    document = new TiXmlDocument(policy_file_.c_str());
    if(!document)
    {
        return -1;
        LOG_ERROR("ReadPolicy: document is NULL.");
    }

    int ret = 0;
    do
    {
        policy_mutex_.Lock();
        if(document->LoadFile() == false)
        {
            ret = -1;
            LOG_ERROR("ReadPolicy: load file failed");
            policy_mutex_.Unlock();
            break;
        }
        policy_mutex_.Unlock();

        root_element = document->RootElement();
        if(root_element == NULL)
        {
            ret = -1;
            LOG_ERROR("ReadPolicy: policy file is empty");
            break;
        }

        string circle_time;
        age_element = root_element->FirstChildElement("timers");
        if(age_element)
        {
            age_element = age_element->FirstChildElement("timerBean");
            if(age_element)
            {
                age_element = age_element->FirstChildElement("cycle");
                if(age_element != NULL)
                {
                    circle_time = age_element->GetText() ? age_element->GetText() : "";
                    if(circle_time.empty())
                    {
                        LOG_WARN("ReadPolicy: cycle time is empty in policy.xml");
                    }
                    policy_param.update_time = SwitchTime(circle_time);
                }
            }
        }

        age_element = root_element->FirstChildElement("params");
        if(age_element != NULL)
        {
            age_element = age_element->FirstChildElement();
            while(age_element != NULL)
            {
                string key, value;
                key = age_element->Attribute("key");
                value = age_element->Attribute("value");
                if(key == "intervalTime")
                    policy_param.interval_time = value;
                if(key == "ipRange")
                {
                    child_element = age_element->FirstChildElement();
                    while(child_element)
                    {
                        string ip, orgId;
                        TiXmlElement  * element = NULL;
                        element = child_element->FirstChildElement("ip");
                        if(element)
                        {
                            ip = element->GetText() ? element->GetText() : "";
                        }

                        element = child_element->FirstChildElement("orgId");
                        if(element)
                        {
                            orgId = element->GetText() ? element->GetText() : "";
                        }

                        if(!orgId.empty() && !ip.empty())
                        {
                            policy_param.ip_range.insert(std::pair<string, string>(ip, orgId));
                        }
                        child_element = child_element->NextSiblingElement();
                    }
                }
                age_element = age_element->NextSiblingElement();
            }
        }
        else
        {
            ret = -1;
            LOG_ERROR("ReadPolicy: params is empty in policy.xml");
            break;
        }

        if(policy_param.interval_time.empty())
            policy_param.interval_time = DEFAULT_INTERVAL; //60S
        if(circle_time.empty())
            policy_param.update_time = DEFAULT_UPDATE_TIME;

        /*string key, value;
        key = "service.serverAreaId";
        if(ParseConfigure::GetInstance().GetProperty(key, value) && !value.empty())
        {
            policy_param.area_id = value;
        }
        else
        {
            policy_param.area_id = DEFAULT_AREA_ID;
        }*/

    }while(0);

    if(!document)
    {
        delete document;
    }
    return ret;
}

bool ParsePolicy::GetPolicyCrc(std::string& crc)
{
    if(policy_file_.empty())
    {
        LOG_WARN("GetPolicyCrc: ParsePolicy not been init.");
        policy_file_ = POLICY_FILE;
    }
    policy_mutex_.Lock();
    FILE* fp = fopen(policy_file_.c_str(), "r");
    if(fp == NULL)
    {
        policy_mutex_.Unlock();
        LOG_ERROR("GetPolicyCrc: open file "<<policy_file_<<" failed.");
        return false;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = new char[size + 1];
    memset(buf, 0, size + 1);
    
    fread(buf, size, 1, fp);
    crc = calCRC(buf);
    delete[] buf;
    fclose(fp);
    policy_mutex_.Unlock();
    return true;
}

int ParsePolicy::WritePolicy(const std::string& data)
{
    if(data.empty())
    {
        LOG_ERROR("WritePolicy: data is empty.");
        return -1;
    }

    if(policy_file_.empty())
    {
        LOG_WARN("GetPolicyCrc: ParsePolicy not been init.");
        policy_file_ = POLICY_FILE;
    }

    policy_mutex_.Lock();
    FILE* fp = fopen(policy_file_.c_str(), "w+");
    if(fp == NULL)
    {
        LOG_ERROR("WritePolicy: open file "<<policy_file_<<" failed.");
        return -1;
    }

    int size = fwrite(data.c_str(), data.size(), 1, fp);
    fclose(fp);
    policy_mutex_.Unlock();
    return size;
}
}}}
