#ifndef PARSE_POLICY_H_
#define PARSE_POLICY_H_

#include "mutex.h"
#include <map>
#include <string>

namespace cems{ namespace service{ namespace scan{

#define POLICY_FILE "../config/policy.xml"
#define DEFAULT_INTERVAL        "60" // 60 S
#define DEFAULT_UPDATE_TIME     600 //10 minits
#define DEFAULT_AREA_ID         "serverAreaMain"

typedef struct
{
    std::map<std::string, std::string> ip_range;
    std::string interval_time;
    long long update_time;
} PolicyParam;

class ParsePolicy
{
public:
    static ParsePolicy& GetInstance();
    void Init(const std::string& file_name = POLICY_FILE);
    ~ParsePolicy();

    int ReadPolicy(PolicyParam& policy_param);
    int WritePolicy(const std::string& data);
    bool GetPolicyCrc(std::string& crc);

private:
    ParsePolicy();
    long long SwitchTime(std::string cycle);

private:
    Mutex policy_mutex_;
    std::string policy_file_;
};
}}}
#endif // PARSE_POLICY_H_
