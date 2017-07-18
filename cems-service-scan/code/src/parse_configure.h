#ifndef _PARSE_CONFIGURE_H_
#define _PARSE_CONFIGURE_H_

#include <map>
#include <string>
#include <mutex>

namespace cems{ namespace service{ namespace scan{

#define COMMENT_CHAR '#'
#define CONFIG_PATH "../config/config.properties"

class ParseConfigure
{
public:
    ~ParseConfigure();
    static ParseConfigure& GetInstance();
    void Init(const std::string& filename = CONFIG_PATH);
    bool GetProperty(const std::string& key, std::string& value);
    void SetProperty(const std::string& key, const std::string& value, bool flush = true);
    void Flush();
    void PrintConfig(); //for test

private:
    ParseConfigure();
    bool ReadConfig(const std::string& filename);
    bool AnalyseLine(const std::string& line, std::string& key, std::string& value);
    bool IsCommentChar(char c);
    void Trim(std::string & str);
    bool IsSpace(char c);
    void UpdateConfigFile(const std::string& key, const std::string& value, bool flush);
    void AppendToConfigFile(const std::string& key, const std::string& value, bool flush);

private:
    std::string config_file_;
    std::map<std::string, std::string> m_configure_;
    std::mutex mutex_file_;

};
}}}
#endif // _PARSE_CONFIGURE_H_
