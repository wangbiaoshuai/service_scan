//英文原义：ADDRESS RESOLUTION PROTOCOL
//中文释义：（RFC - 826）地址解析协议
#include "arp.h"

#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
using namespace std;

#include "log.h"

#define device "eth0"                //本机的哪块网卡
#define fill_buf "aaaaaaaaaaaa"
int socket_id;
char *target = NULL;
struct in_addr  src, dst;
struct sockaddr_ll   me, he;
struct in_addr get_src_ip(const char * devices)//获得本机相应网卡的ip
{
    struct sockaddr_in saddr;
    int sock_id = socket(AF_INET, SOCK_DGRAM, 0);//设置数据报socket
    if (sock_id < 0) 
    {
        LOG_ERROR("socket: "<<strerror(errno));
        return saddr.sin_addr;
    }
    if (devices) 
    {
        if (setsockopt(sock_id, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device) + 1) == -1)
        {
            //将socketbind到网卡上
            LOG_WARN("get_src_ip: interface is ignored.");
        }
    }
    int alen = sizeof(saddr);
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_port = htons(0x1000);//设置端口
    saddr.sin_family = AF_INET;
    if (connect(sock_id, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) 
    {
        //将socket连接到相应的inet地址上
        LOG_ERROR("connect: "<<strerror(errno));
        return saddr.sin_addr;
    }
    if (getsockname(sock_id, (struct sockaddr*)&saddr, (socklen_t*)&alen) == -1) 
    {
        //通过socket获得绑定的ip地址
        LOG_ERROR("getsockname: "<<strerror(errno));
        return saddr.sin_addr;
    }
    close(sock_id);
    return saddr.sin_addr;
}
int check_device(const char* if_dev, int ss)//网卡和socket    将网卡设置为混杂模式？
{
    int ifindex;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_dev, IFNAMSIZ - 1);//网卡设备名
    if (ioctl(ss, SIOCGIFINDEX, &ifr) < 0) 
    {
        LOG_ERROR("arping: unknow iface "<<if_dev);
        return -1;
    }
    ifindex = ifr.ifr_ifindex;
    if (ioctl(ss, SIOCGIFFLAGS, (char*)&ifr)) 
    {
        LOG_ERROR("ioctl(SIOCGIFFLAGS): "<<strerror(errno));
        return -1;
    }
    if (!(ifr.ifr_flags&IFF_UP)) 
    {
        LOG_ERROR("interface "<<if_dev<<"is down.");
        return -1;
    }
    if (ifr.ifr_flags&(IFF_NOARP | IFF_LOOPBACK)) 
    {
        LOG_ERROR("interface "<<if_dev<<" is not ARPable");
        return -1;
    }
    return ifindex;
} // check_device()

int socket_init()
{
    int s, s_errno;
    s = socket(PF_PACKET, SOCK_DGRAM, 0);//数据包
    s_errno = errno;
    me.sll_family = AF_PACKET;
    me.sll_ifindex = check_device(device, s);
    me.sll_protocol = htons(ETH_P_ARP);
    if (bind(s, (struct sockaddr*)&me, sizeof(me)) == -1) 
    {
        LOG_ERROR("bind: "<<strerror(errno));
        return -1;
    }
    int alen = sizeof(me);
    if (getsockname(s, (struct sockaddr*)&me, (socklen_t*)&alen) == -1) 
    {
        LOG_ERROR("getsockname: "<<strerror(errno));
        return -1;
    }
    if (me.sll_halen == 0) 
    {
        LOG_ERROR("interface "<<device<<" is not ARPable (no ll address)");
        return -1;
    }
    he = me;
    memset(he.sll_addr, -1, he.sll_halen);  // set dmac addr FF:FF:FF:FF:FF:FF
    return s;
}
int
create_pkt(unsigned char * buf, struct in_addr src, struct in_addr dst, struct sockaddr_ll * FROM, struct sockaddr_ll * TO)
{
    struct arphdr *ah = (struct arphdr*) buf;
    unsigned char *p = (unsigned char *)(ah + 1);
    ah->ar_hrd = htons(FROM->sll_hatype);
    if (ah->ar_hrd == htons(ARPHRD_FDDI))
        ah->ar_hrd = htons(ARPHRD_ETHER);
    ah->ar_pro = htons(ETH_P_IP);
    ah->ar_hln = FROM->sll_halen;
    ah->ar_pln = 4;
    ah->ar_op = htons(ARPOP_REQUEST);
    memcpy(p, &FROM->sll_addr, ah->ar_hln);
    p += FROM->sll_halen;
    memcpy(p, &src, 4);
    p += 4;
    memcpy(p, &TO->sll_addr, ah->ar_hln);
    p += ah->ar_hln;
    memcpy(p, &dst, 4);
    p += 4;
    memcpy(p, fill_buf, strlen(fill_buf));
    p += 12;
    return  (p - buf);
}
void send_pkt()
{
    int send_count = 0;
    unsigned char send_buf[256];
    int pkt_size = create_pkt(send_buf, src, dst, &me, &he);
    if(pkt_size <= 0)
        return;
    do
    {
        int cc = sendto(socket_id, send_buf, pkt_size, 0, (struct sockaddr*)&he, sizeof(he));
        if (cc == pkt_size)
        {
            break;
        }
        else
        {
            send_count++;
        }
    }while(send_count < 4);
    return;
}
int chk_recv_pkt(unsigned char * buf, struct sockaddr_ll * FROM)//解析arp数据
{
    struct arphdr *ah = (struct arphdr*)buf;
    unsigned char *p = (unsigned char *)(ah + 1);
    struct in_addr src_ip, dst_ip;
    if (ah->ar_op != htons(ARPOP_REQUEST) && ah->ar_op != htons(ARPOP_REPLY))
        return 0;
    if (ah->ar_pro != htons(ETH_P_IP) || ah->ar_pln != 4 || ah->ar_hln != me.sll_halen)
        return 0;
    memcpy(&src_ip, p + ah->ar_hln, 4);
    memcpy(&dst_ip, p + ah->ar_hln + 4 + ah->ar_hln, 4);
    if (src_ip.s_addr != dst.s_addr || src.s_addr != dst_ip.s_addr)
        return 0;
    return (p - buf);
}

int get_mac_addr(const std::string& ip, std::string& mac)
{
    if(ip.empty())
    {
        LOG_ERROR("get_mac_addr: target ip is NULL.");
        return -1;
    }

    target = (char*)ip.c_str();
    
    if (inet_aton(target, &dst) != 1) 
    {
        //使用字符串ip更新dst地址结构中的网络字节序ip
        struct hostent *hp;
        hp = gethostbyname2(target, AF_INET);
        if (!hp) 
        {
            LOG_ERROR("get_mac_addr: unknow host "<<target);
            return -1;
        }
        memcpy(&dst, hp->h_addr, 4);
    }
    
    src = get_src_ip(device);//获得本机device网卡的ip
    if (!src.s_addr) 
    {
        LOG_ERROR("get_mac_addr: no source address in not-DAD node");
        return -1;
    }

    socket_id = socket_init();
    LOG_DEBUG("arping "<<target<<" from "<<inet_ntoa(src)<<" "<<device);

    int ret = -1;
    int recv_count = 0;
    do 
    {
        struct sockaddr_ll from;
        int alen = sizeof(from);
        char recv_buf[0x1000];
        send_pkt();
        int recv_size = recvfrom(socket_id, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&from, (socklen_t*)&alen);

        if (recv_size < 0) 
        {
            LOG_ERROR("get_mac_addr: recvfrom "<<strerror(errno));
            return -1;;
        }
        
        if (chk_recv_pkt((unsigned char*)recv_buf, &from) > 0) 
        {
            memcpy(he.sll_addr, from.sll_addr, he.sll_halen);
            char buffer[20] = {0};
            sprintf(buffer, "%02X-%02X-%02X-%02X-%02X-%02X", from.sll_addr[0], from.sll_addr[1], from.sll_addr[2], from.sll_addr[3], from.sll_addr[4], from.sll_addr[5]);
            mac = buffer;
            LOG_DEBUG("get "<<target<<" mac: "<<mac.c_str());
            ret = 0;
            break;
        }
        else
        {
            recv_count++; 
        }
    }while(recv_count < 4);

    return ret;
}

/*int main(void)
{
    INIT_LOG("../config/log4cplus.properties");
    string mac;
    const char* ip = "192.168.134.147";
    get_mac_addr(ip, mac);
    printf("%s mac: %s\n", ip, mac.c_str());
    return 0;
}*/
