#include "common.h"
#include "test.h"
#include <stdio.h>
#include <string>
using namespace std;

Test::Test():
parse_router_()
{
}

Test::~Test()
{
}

void Test::Get()
{
    string key = "00FF0100:00FF1100";
    string value;
    parse_router_.GetRouter(key, value);
}

int main()
{
    printf("%d\n", sizeof(unsigned int));
    printf("%d\n", sizeof(WORD));
    printf("%d\n", sizeof(CEMS_NET_HEAD));

    char x = '1';
    char buf[9] = {9};
    string maxcode("");
    sprintf(buf, "%08X", x);
    maxcode = string(buf);
    printf("%s\n", maxcode.c_str());

    Test test;
    test.Get();

    return 0;
}
