#include "parse_router.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>

using namespace std;

const static char* ROUTER_FILE = "./router.ini";

ParseRouter::ParseRouter():
router_map_()
{
    ReadFile(ROUTER_FILE);
}

ParseRouter::~ParseRouter()
{
}

int ParseRouter::ReadFile(const std::string& file)
{
    if(file.empty())
    {
        printf("file path is NULL.\n");
        return -1;
    }

    FILE* fp = fopen(file.c_str(), "r");
    if(fp == NULL)
    {
        printf("open file error: %s\n", strerror(errno));
        return -1;
    }
    while(true)
    {
        char buffer[255] = {0};
        fgets(buffer, sizeof(buffer), fp);
        if(feof(fp))
        {
            break;
        }

        if(strlen(buffer) == 0 || buffer[0] == '#')
        {
            continue;
        }

        std::string key;
        std::string value;
        int equal_num = 0;
        bool start_str = false;
        for(unsigned int i = 0; i < strlen(buffer); i++)
        {
            if(buffer[i] == '=')
            {
                start_str = true;
                equal_num++;
                continue;
            }
            if(buffer[i] == ' ' || buffer[i] == '\n')
            {
                start_str = false;
                continue;
            }

            if(start_str)
            {
                if(equal_num == 1)
                {
                    key += buffer[i];
                }
                else if(equal_num == 2)
                {
                    value += buffer[i];
                }
            }
        }
        router_map_.insert(pair<string, string>(key, value));
    }

    fclose(fp);
    return 0;
}

bool ParseRouter::GetRouter(const string& key, string& value)
{
    if(key.empty() || router_map_.empty())
    {
        return false;
    }

    map<string, string>::iterator iter = router_map_.find(key);
    if(iter == router_map_.end())
    {
        return false;
    }
    else
    {
        value = router_map_[key];
    }

    // test
    /*for(iter = router_map_.begin(); iter != router_map_.end(); ++iter)
    {
        printf("key: %s, value: %s\n", iter->first.c_str(), iter->second.c_str());
    }*/
    return true;
}

/*
int main()
{
    string key = "00FF1300:00FF0001";
    string value;
    ParseRouter parse_router;
    parse_router.GetRouter(key, value);
    printf("key=%s, value=%s\n", key.c_str(), value.c_str());
    return 0;
}
*/
