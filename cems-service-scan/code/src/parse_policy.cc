#include "parse_policy.h"

#include <errno.h>
#include "log.h"
#include "tinyxml.h"
#include "common_function.h"

namespace cems{ namespace service{ namespace scan{
using namespace std;

#define LOG_CONFIG_PATH "../config/log4cplus.properties"

ParsePolicy::ParsePolicy() :
policy_mutex_(),
policy_file_(""),
current_log_level_("DEBUG")
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

/*        age_element = root_element->FirstChildElement("logBean");
        if(age_element != NULL)
        {
            string log_level = age_element->Attribute("logLevel");
            if(log_level != current_log_level_)
            {
                SetLogLevel(LOG_CONFIG_PATH, log_level);
                current_log_level_ = log_level;
            }
        }*/

        if(policy_param.interval_time.empty())
            policy_param.interval_time = DEFAULT_INTERVAL; //60S
        if(circle_time.empty())
            policy_param.update_time = DEFAULT_UPDATE_TIME;

    }while(0);

    if(!document)
    {
        delete document;
    }
    return ret;
}

int ParsePolicy::SetLogLevel()
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

        age_element = root_element->FirstChildElement("logBean");
        if(age_element != NULL)
        {
            string log_level = age_element->Attribute("logLevel");
            if(log_level != current_log_level_)
            {
                SetLogLevel(LOG_CONFIG_PATH, log_level);
                current_log_level_ = log_level;
            }
        }
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
        policy_mutex_.Unlock();
        LOG_ERROR("WritePolicy: open file "<<policy_file_<<" failed.");
        return -1;
    }

    int size = fwrite(data.c_str(), data.size(), 1, fp);
    fclose(fp);
    policy_mutex_.Unlock();
    return size;
}

const char* LOG_LEVEL_SET_STR = "log4cplus.appender.AppenderName.filters.1.LogLevelMin";
int ParsePolicy::SetLogLevel(const string&  log_config_file, string log_level)
{
    log4cplus::LogLevel level;

    if(log_level == "TRACE")
    {
        level = log4cplus::TRACE_LOG_LEVEL;
    }
    else if (log_level == "DEBUG")
    {
        level = log4cplus::DEBUG_LOG_LEVEL;
    }
    else if(log_level == "INFO")
    {
        level = log4cplus::INFO_LOG_LEVEL;
    }
    else if(log_level == "WARN")
    {
        level = log4cplus::WARN_LOG_LEVEL;
    }
    else if(log_level == "ERROR")
    {
        level = log4cplus::ERROR_LOG_LEVEL;
    }
    else if(log_level == "FATAL")
    {
        level = log4cplus::FATAL_LOG_LEVEL;
    }
    else
    {
        log_level = "TRACE";
        level = log4cplus::TRACE_LOG_LEVEL;
    }

    FILE* fp = fopen(log_config_file.c_str(), "r+");
    if(fp == NULL)
    {
        LOG_ERROR("open "<<log_config_file.c_str()<<" error:"<<strerror(errno));
        return -1;
    }

    int pos = 0;
    while(!feof(fp))
    {
        char line[512] = {0};
        fgets(line, sizeof(line), fp);
        if(feof(fp))
            break;

        if(strncmp(line, LOG_LEVEL_SET_STR, strlen(LOG_LEVEL_SET_STR)) == 0)
        {
            int offset = ftell(fp) - pos;
            fseek(fp, -offset, SEEK_CUR);
            char data[200] = {0};
            sprintf(data, "%s=%-5s", LOG_LEVEL_SET_STR, log_level.c_str());
            fputs(data, fp);
            break;
        }
        pos = ftell(fp);
    }
    fclose(fp);
    log4cplus::Logger::getRoot().setLogLevel(level);
    LOG_DEBUG("SetLogLevel: "<<log_level.c_str());
    return 0;
}
}}}
