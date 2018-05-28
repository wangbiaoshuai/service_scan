#ifndef TEST_H
#define TEST_H

#include "parse_router.h"

class Test
{
public:
    Test();
    ~Test();
    void Get();

private:
    ParseRouter parse_router_;
};
#endif // TEST_H
