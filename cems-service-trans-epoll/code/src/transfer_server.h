#ifndef _TRANSFER_SERVER_H
#define _TRANSFER_SERVER_H

#include <string>
#include <map>

typedef struct _PARAM
{
    int sock_;
    void* context_;
} Param;

typedef struct _MSG_BODY
{
    std::string session_id_;
    std::string src_device_id_;
    std::string dst_device_id_;
    std::string device_type_;
    _MSG_BODY()
    {
        session_id_ = "";
        src_device_id_ = "";
        dst_device_id_ = "";
        device_type_ = "";
    }
} MsgBody;

typedef struct _MATCH_INFO
{
    int sock_;
    int dst_sock_;
    bool is_matched_;
    bool timeout_;
    unsigned int header_flag_;
    unsigned int header_version_;
    unsigned int header_msgcode_;
    unsigned int header_maxcode_;
    unsigned int header_mincode_;
    unsigned short header_type_;
    unsigned short header_count_;
    unsigned short header_index_;
    MsgBody msg_body_;
    _MATCH_INFO():
    msg_body_()
    {
        sock_ = -1;
        dst_sock_ = -1;
        is_matched_ = false;
        timeout_ = false;
        header_flag_ = 0;
        header_version_ = 0;
        header_msgcode_ = 0;
        header_maxcode_ = 0;
        header_mincode_ = 0;
        header_type_ = 0;
        header_count_ = 0;
        header_index_ = 0;
    }
} MatchInfo;

class TransferServer
{
public:
    TransferServer(int port);
    ~TransferServer();

    int Start();
    int StartServer();
    int TryMatch(int);

private:
    int InitSocket();
    int WriteResponse(int);

private:
    int port_;
    int listen_sock_;

public:
    int epfd_;
    std::map<int, MatchInfo> session_map_;
};
#endif // _TRANSFER_SERVER_H
