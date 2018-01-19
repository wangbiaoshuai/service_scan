#ifndef		DEFINES_H_
#define		DEFINES_H_

#include <string>

#define CONFIG_CONFIG_PROPERTIES   "config.properties"

struct ST_ENCRYPT
{
        ST_ENCRYPT()
        {
                mode = 0;
                szKey = "";
        }
        int mode;
        std::string szKey;
};
#endif
