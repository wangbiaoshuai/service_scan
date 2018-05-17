#ifndef PARSE_ROUTER_H
#define PARSE_ROUTER_H

#include <string>
#include <map>

class ParseRouter
{
public:
    ParseRouter();
    ~ParseRouter();
    bool GetRouter(const std::string&, std::string&);

private:
    int ReadFile(const std::string&);

private:
    std::map<std::string, std::string> router_map_;

};
#endif // PARSE_ROUTER_H
