#include "transfer_server.h"

#include <string>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "thread_pool.h"
#include "log.h"
#include "parse_configure.h"
#include "json/json.h"
#include "common.h"

using namespace std;
using namespace cems::service::scan;

#define WAIT_Q 10
#define EPOLL_Q 1000
#define EVENT_NUM 1000
#define THREAD_POOL_NUM 10
#define MATCH_COUNT 45
#define RECV_TIMEOUT 10

TransferServer::TransferServer(int port):
port_(port),
listen_sock_(-1),
epfd_(-1),
session_map_()
{
}

TransferServer::~TransferServer()
{
}

void* transfer_function(void* context)
{
    TransferServer* server = (TransferServer*)context;
    server->StartServer();
    pthread_exit(NULL);
}

int TransferServer::Start()
{
    pthread_t serv_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int res = pthread_create(&serv_thread, &attr, transfer_function, this);
    if(res < 0)
    {
        LOG_ERROR("Start: create thread failed.");
        return -1;
    }
    pthread_attr_destroy(&attr);
    return 0;
}

int TransferServer::InitSocket()
{
    string local_ip;
    if(ParseConfigure::GetInstance().GetProperty("service.ip", local_ip) == false || local_ip.empty())
    {
        LOG_ERROR("InitSocket: get local ip failed.");
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        LOG_ERROR("InitSocket: create socket error("<<strerror(errno)<<").");
        return -1;
    }

    int opt = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(res < 0)
    {
        LOG_ERROR("InitSocket: set reuse addr error("<<strerror(errno)<<").");
        return -1;
    }

    LOG_DEBUG("InitSocket: port="<<port_);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = inet_addr(local_ip.c_str());

    res = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(res < 0)
    {
        LOG_ERROR("InitSocket: bind error("<<strerror(errno)<<").");
        return -1;
    }

    res = listen(sock, WAIT_Q);
    if(res < 0)
    {
        LOG_ERROR("InitSocket: listen error("<<strerror(errno)<<").");
        return -1;
    }

    LOG_INFO("InitSocket: listen on "<<port_);
    return sock;
}

void set_nonblock(int sock)
{
    int fl = fcntl(sock, F_GETFL);
    fcntl(sock, F_SETFL, fl | O_NONBLOCK);
}

int parse_header(unsigned int flag)
{
    char* ptr = (char*)&flag;
    if(*ptr != '_')
        return -1;
    ptr++;

    if(*ptr != 'e')
        return -1;
    ptr++;

    if(*ptr != 'd')
        return -1;
    ptr++;

    if(*ptr != 'p')
        return -1;
    return 0;
}

int parse_body(char* match_body, int msg_len, MsgBody* msg_body)
{
    if(match_body == NULL || msg_body == NULL || msg_len <= 0)
        return -1;

    string json(match_body, msg_len);
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(json, root))
    {
        if(root.isMember("SessionID") && root.isMember("SrcDeviceID") && root.isMember("DstDeviceID") && root.isMember("DeviceType"))
        {
            if(root["SessionID"].isString() && root["SrcDeviceID"].isString() && root["DstDeviceID"].isString() && root["DeviceType"].isString())
            {
                msg_body->session_id_ = root["SessionID"].asString();
                msg_body->src_device_id_ = root["SrcDeviceID"].asString();
                msg_body->dst_device_id_ = root["DstDeviceID"].asString();
                msg_body->device_type_ = root["DeviceType"].asString();
                return 0;
            }
        }
    }
    return -1;
}

int TransferServer::TryMatch(int sock)
{
    if(session_map_[sock].is_matched_)
    {
        LOG_INFO("TryMatch: session("<<session_map_[sock].msg_body_.session_id_<<") has been matched.");
        return 0;
    }
    map<int, MatchInfo>::iterator iter;
    for(iter = session_map_.begin(); iter != session_map_.end(); ++iter)
    {
        if(iter->second.is_matched_)
        {
            continue;
        }
        else
        {
            if(iter->second.msg_body_.session_id_ == session_map_[sock].msg_body_.session_id_
               && iter->second.msg_body_.src_device_id_ == session_map_[sock].msg_body_.dst_device_id_
               && iter->second.msg_body_.dst_device_id_ == session_map_[sock].msg_body_.src_device_id_ 
               && iter->second.msg_body_.device_type_ != session_map_[sock].msg_body_.device_type_)
            {
                iter->second.is_matched_ = true;
                iter->second.dst_sock_ = sock;
                session_map_[sock].is_matched_ = true;
                session_map_[sock].dst_sock_ = iter->second.sock_;
                LOG_INFO("TryMatch: session("<<iter->second.msg_body_.session_id_<<") is matched.");
                return 0;
            }

        }
    }
    return -1;
}

void* process_msg(void* args)
{
    int sock = ((Param*)args)->sock_;
    TransferServer* context = (TransferServer*)(((Param*)args)->context_);
    int epfd = context->epfd_;
    map<int, MatchInfo>::iterator iter = context->session_map_.find(sock);
    char buffer[1024] = {0};
    if(iter == context->session_map_.end())
    {
        MatchInfo match_info;
        int res = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if(res < 0)
        {
            LOG_ERROR("recv header error("<<strerror(errno)<<").");
            close(sock);
            return NULL;
        }
        else if(res == 0)
        {
            LOG_ERROR("socket closed");
            close(sock);
            return NULL;
        }

        PCEMS_NET_HEAD p_header = (PCEMS_NET_HEAD)buffer;
        match_info.sock_ = sock;
        match_info.header_maxcode_ = p_header->dwMaxCode;
        match_info.header_mincode_ = p_header->dwMinCode;
        match_info.header_flag_ = p_header->dwFlag;
        match_info.header_version_ = p_header->dwVersion;
        match_info.header_msgcode_ = p_header->dwMsgCode;
        match_info.header_type_ = p_header->wType;
        match_info.header_count_ = p_header->wCount;
        match_info.header_index_ = p_header->wIndex;

        if(parse_header(match_info.header_flag_) < 0)
        {
            LOG_ERROR("parse header error, close sock.");
            close(sock);
            return NULL;
        }

        memset(buffer, 0, sizeof(buffer));
        for(int i = 0; i < RECV_TIMEOUT; i++)
        {
            res = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if(res < 0)
            {
                if(errno == EAGAIN)
                {
                    if(i == RECV_TIMEOUT - 1)
                    {
                        LOG_ERROR("recv match body timeout, close socket.");
                        close(sock);
                        return NULL;
                    }
                    sleep(1);
                    continue;
                }
                LOG_ERROR("recv match body error("<<strerror(errno)<<").");
                close(sock);
                return NULL;
            }
            else if(res == 0)
            {
                LOG_ERROR("socket closed");
                close(sock);
                return NULL;
            }
            break;
        }

        if(parse_body(buffer, strlen(buffer), &(match_info.msg_body_)) < 0)
        {
            LOG_ERROR("parse match body error, close sock.");
            close(sock);
            return NULL;
        }
        context->session_map_.insert(std::pair<int, MatchInfo>(sock, match_info));

        int i;
        for(i = 0; i < MATCH_COUNT; i++)
        {
            if(context->TryMatch(sock) == 0) // 需要实现
            {
                LOG_INFO("session:"<<match_info.msg_body_.session_id_<<" matched...");
                break;
            }
            sleep(1);
        }
        if(i == MATCH_COUNT)
        {
            context->session_map_[sock].timeout_ = true;
            context->session_map_[sock].is_matched_ = false;
        }
        else
        {
            context->session_map_[sock].timeout_ = false;
            context->session_map_[sock].is_matched_ = true;
        }

        epoll_event event;
        event.events = EPOLLOUT | EPOLLET;
        event.data.fd = sock;
        epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &event);
    }
    else
    {
        if(context->session_map_[sock].is_matched_)
        {
            int res_size = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if(res_size < 0)
            {
                LOG_ERROR("recv messege error("<<strerror(errno)<<").");
                close(sock);
                return NULL;
            }
            else if(res_size == 0)
            {
                LOG_ERROR("client socket close.");
                close(sock);
                return NULL;
            }
            int dst_sock = context->session_map_[sock].dst_sock_;
            int res = send(dst_sock, buffer, res_size, 0);
            if(res < 0)
            {
                LOG_ERROR("send messege error("<<strerror(errno)<<").");
                close(sock);
                return NULL;
            }
            else if(res == 0)
            {
                LOG_ERROR("client socket close.");
                close(sock);
                return NULL;
            }
            return NULL;
        }
        else
        {
            LOG_ERROR("error event, close socket.");
            close(sock);
            return NULL;
        }
    }
    return NULL;
}

int TransferServer::StartServer()
{
    LOG_DEBUG("StartServer: begin.");
    listen_sock_ = InitSocket();
    if(listen_sock_ < 0)
    {
        LOG_ERROR("StartServer: InitSocket failed.");
        return -1;
    }
    set_nonblock(listen_sock_);

    epfd_ = epoll_create(EPOLL_Q);
    if(epfd_ < 0)
    {
        LOG_ERROR("StartServer: create epoll error("<<strerror(errno)<<").");
        return -1;
    }

    struct epoll_event event;
    struct epoll_event evs[EVENT_NUM];
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listen_sock_;
    epoll_ctl(epfd_, EPOLL_CTL_ADD, listen_sock_, &event);

    ThreadPool thread_pool;
    if(thread_pool.Init(THREAD_POOL_NUM) < 0)
    {
        LOG_ERROR("StartServer: thread pool init failed.");
        return -1;
    }

    while(true)
    {
        LOG_DEBUG("StartServer: epoll begin wait...");
        int event_num = epoll_wait(epfd_, evs, EVENT_NUM, -1);
        if(event_num < 0)
        {
            LOG_ERROR("StartServer: epoll wait error("<<strerror(errno)<<").");
            break;
        }
        else if(event_num == 0)
        {
            LOG_ERROR("StartServer epoll wait timeout.");
            continue;
        }
        else
        {
            for(int i = 0; i < event_num; i++)
            {
                int rsock = evs[i].data.fd;
                if(rsock == listen_sock_ && (evs[i].events & EPOLLIN))
                {
                    struct sockaddr_in client_addr = {0};
                    socklen_t len = sizeof(client_addr);
                    int client_sock = accept(listen_sock_, (struct sockaddr*)&client_addr, &len);
                    if(client_sock < 0)
                    {
                        LOG_ERROR("StartServer: accept error("<<strerror(errno)<<").");
                        continue;
                    }
                    set_nonblock(client_sock);
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    event.data.fd = client_sock;
                    epoll_ctl(epfd_, EPOLL_CTL_ADD, client_sock, &event);
                }
                else if(evs[i].events & EPOLLIN)
                {
                    Param param;
                    param.sock_ = rsock;
                    param.context_ = this;
                    thread_pool.AddJob(process_msg, &param);
                }
                else if(evs[i].events & EPOLLOUT)
                {
                    map<int, MatchInfo>::iterator iter = session_map_.find(rsock);
                    if(iter != session_map_.end())
                    {
                        bool is_matched = session_map_[rsock].is_matched_;
                        if(is_matched)
                        {
                            if(WriteResponse(rsock) < 0)
                            {
                                LOG_ERROR("StartServer: WriteResponse failed, close socket.");
                                close(rsock);
                                continue;
                            }
                            event.data.fd = rsock;
                            event.events = EPOLLIN | EPOLLET;
                            epoll_ctl(epfd_, EPOLL_CTL_MOD, rsock, &event);
                        }
                        else
                        {
                            LOG_WARN("StartServer: session("<<session_map_[rsock].msg_body_.session_id_<<") not matched, close socket.");
                            session_map_.erase(rsock);
                            close(rsock);
                            continue;
                        }
                    }
                    else
                    {
                        LOG_ERROR("StartServer: event error, close socket.");
                        close(rsock);
                        continue;
                    }
                }
            }
        }
    }

    thread_pool.Destroy();
    close(epfd_);
    close(listen_sock_);
    return 0;
}

int TransferServer::WriteResponse(int sock)
{
    LOG_DEBUG("WriteResponse: write to "<<sock);
    map<int, MatchInfo>::iterator iter = session_map_.find(sock);
    if(iter == session_map_.end())
    {
        LOG_ERROR("WriteResponse: the socket not in session map.");
        return -1;
    }

    string result = "0";
    string description = "Session matched.";
    
    stringstream ss;
    ss<<"{\"maxCode\":\""<<iter->second.header_maxcode_<<"\",\"minCode\":\""<<iter->second.header_mincode_<<"\",\"result\":\" "<<result<<"\",\"description\":\""<<description<<"\"}";
    string response_str = ss.str();

    CEMS_NET_HEAD header;
    header.dwDataSize = response_str.length() + 1;
    header.dwFlag = iter->second.header_flag_;
    header.dwVersion = iter->second.header_version_;
    header.dwMsgCode = iter->second.header_msgcode_;
    header.dwMaxCode = iter->second.header_maxcode_;
    header.dwMinCode = iter->second.header_mincode_;
    header.wHeadSize = sizeof(CEMS_NET_HEAD);
    header.wType = iter->second.header_type_;
    header.wCount = iter->second.header_count_;
    header.wIndex = iter->second.header_index_;

    int res = send(sock, &header, sizeof(header), 0);
    if(res <= 0)
    {
        LOG_ERROR("WriteResponse: send response header error("<<strerror(errno)<<").");
        return -1;
    }

    res = send(sock, response_str.c_str(), response_str.length() + 1, 0);
    if(res <= 0)
    {
        LOG_ERROR("WriteResponse: send response body error("<<strerror(errno)<<").");
        return -1;
    }    
    return 0;
}
