#ifndef FASTSCAN_H_
#define FASTSCAN_H_

#include <pthread.h>
#include <string>
#include <map>
namespace cems{ namespace service{ namespace scan{

typedef std::map<std::string, std::string> MAP_STRING;
#define MAXCODE_SCAN_SERVICE        "00FF1100"                  //扫描服务
#define MINCODE_SCAN_POLICY         "1000"                    //服务策略更新接口
#define MINCODE_SCAN_NOTIFY         "1001"          //服务区域发生变化，重新注册

class FastScan
{
public:
    FastScan();
    ~FastScan();

    int Start();
    int Stop();
 
    int StartScan();
    int StartUpdatePolicy();

protected:
    long long SwitchTime(std::string cycle);

private:
    int CompareMap(const MAP_STRING& map1, const MAP_STRING& map2);
    int ParseIpRange(const std::string& ip_range, MAP_STRING& range);

private:
    pthread_t scan_thread_;
    pthread_t update_policy_thread_;
    bool stop_scan_;
    bool stop_update_policy_;
};
}}}
#endif // FASTSCAN_H_
