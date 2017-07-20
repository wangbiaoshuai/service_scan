#include "parse_configure.h"

#include <fstream>
#include <iostream>

namespace cems{ namespace service{ namespace scan{
using namespace std;

ParseConfigure::ParseConfigure() :
config_file_(CONFIG_PATH),
m_configure_(),
mutex_file_()
{
}

ParseConfigure::~ParseConfigure()
{
}

ParseConfigure& ParseConfigure::GetInstance()
{
    static ParseConfigure parse_configure;
    return parse_configure;
}

void ParseConfigure::Init(const string& filename)
{
    if(!filename.empty())
    {
        config_file_ = filename;
    }
    ReadConfig(filename);
}

bool ParseConfigure::ReadConfig(const string & filename)
{
    unique_lock<mutex> lck(mutex_file_);
    m_configure_.clear();
    ifstream infile(filename.c_str());
    if (!infile) 
    {
        //cout << "file open error" << endl;
        return false;
    }
    string line, key, value;
    while (getline(infile, line)) 
    {
        if (AnalyseLine(line, key, value)) 
        {
            m_configure_[key] = value;
        }
    }
    
    infile.close();
    return true;
}

bool ParseConfigure::AnalyseLine(const string& line, string& key, string& value)
{
    if (line.empty())
        return false;
    int start_pos = 0, end_pos = line.size() - 1, pos;
    if ((pos = line.find(COMMENT_CHAR)) != -1) {
        if (0 == pos) {  // 行的第一个字符就是注释字符
            return false;
        }
        end_pos = pos - 1;
    }
    string new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // 预处理，删除注释部分
    
    if ((pos = new_line.find('=')) == -1)
        return false;  // 没有=号
        
    key = new_line.substr(0, pos);
    value = new_line.substr(pos + 1, end_pos + 1- (pos + 1));
    
    Trim(key);
    if (key.empty()) {
        return false;
    }
    Trim(value);
    return true;
}

bool ParseConfigure::IsCommentChar(char c)
{
    switch(c) {
    case COMMENT_CHAR:
        return true;
    default:
        return false;
    }
}

void ParseConfigure::Trim(string & str)
{
    if (str.empty()) {
        return;
    }
    unsigned int i, start_pos, end_pos;
    for (i = 0; i < str.size(); ++i) {
        if (!IsSpace(str[i])) {
            break;
        }
    }
    if (i == str.size()) { // 全部是空白字符串
        str = "";
        return;
    }
    
    start_pos = i;
    
    for (i = str.size() - 1; i >= start_pos; --i) {
        if (!IsSpace(str[i])) {
            break;
        }
    }
    end_pos = i;
    
    str = str.substr(start_pos, end_pos - start_pos + 1);
}

bool ParseConfigure::IsSpace(char c)
{
    if (' ' == c || '\t' == c)
        return true;
    return false;
}

//for test
void ParseConfigure::PrintConfig()
{
    map<string, string>::const_iterator mite = m_configure_.begin();
    for (; mite != m_configure_.end(); ++mite) {
        cout << mite->first << "=" << mite->second << endl;
    }
}

bool ParseConfigure::GetProperty(const string& key, string& value)
{
    if(m_configure_.empty())
    {
        ReadConfig(config_file_);
    }

    if(m_configure_.empty())
    {
        return false;
    }

    map<string, string>::iterator it = m_configure_.find(key);
    if(it != m_configure_.end())
    {
        value = it->second;
        return true;
    }
    
    return false;
}

void ParseConfigure::SetProperty(const string& key, const string& value, bool flush)
{
    if(key.empty() || value.empty())
        return;

    if(m_configure_.empty())
    {
        ReadConfig(config_file_);
    }
    map<string, string>::iterator it = m_configure_.find(key);
    if(it != m_configure_.end())
    {
        UpdateConfigFile(key, value, flush);
    }
    else
    {
        AppendToConfigFile(key, value, flush);
    }
}

void ParseConfigure::UpdateConfigFile(const string& key, const string& value, bool flush)
{
    unique_lock<mutex> lck(mutex_file_);
    if(value == m_configure_[key])
        return;
    m_configure_[key] = value;
 
    if(!flush)
        return;

    ofstream outfile(config_file_);
    if(!outfile)
        return;

    map<string, string>::iterator it;
    for(it = m_configure_.begin(); it != m_configure_.end(); ++it)
    {
        string line = it->first + "=" + it->second;
        outfile<<line<<endl;
    }

    outfile.close();        
    return;
}

void ParseConfigure::AppendToConfigFile(const string& key, const string& value, bool flush)
{
    unique_lock<mutex> lck(mutex_file_);
    m_configure_[key] = value;
 
    if(!flush)
        return;

    ofstream outfile(config_file_, ios::out|ios::app);
    if(!outfile)
        return;

    string line = key + "=" + value;
    outfile<<line<<endl;
    outfile.close();
    return;
}

void ParseConfigure::Flush()
{
    unique_lock<mutex> lck(mutex_file_);
    ofstream outfile(config_file_);
    if(!outfile)
        return;

    map<string, string>::iterator it;
    for(it = m_configure_.begin(); it != m_configure_.end(); ++it)
    {
        string line = it->first + "=" + it->second;
        outfile<<line<<endl;
    }

    outfile.close();        
    return;
}
}}}
