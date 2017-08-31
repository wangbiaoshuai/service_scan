#ifndef _ARP_H
#define _ARP_H
#include <string>
extern "C"{
int get_mac_addr(const std::string& ip, std::string& mac);
}

#endif
