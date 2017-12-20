#include "transferserver.h"
#include <time.h>

#include "typedef.h"
#include "log.h"

#define TIMEOUT_S1 1
#define TIMEOUT_S2 1

/*
template<typename T>
void safe_delete(T* a) 
{
  if(a)
  {
    delete a;
    a = NULL;
  }
}
*/

void print_timestamp()
{
  struct timespec st_tsNow;
  struct tm st_tmLocalTime;
  char szLogBuf[8*1024];   
  clock_gettime(CLOCK_REALTIME, &st_tsNow);
  localtime_r(&st_tsNow.tv_sec, &st_tmLocalTime);
  sprintf(szLogBuf, "%04d/%02d/%02d %02d:%02d:%02d.%03ld \n",
          st_tmLocalTime.tm_year + 1900,
          st_tmLocalTime.tm_mon + 1,
          st_tmLocalTime.tm_mday,
          st_tmLocalTime.tm_hour,
          st_tmLocalTime.tm_min,
          st_tmLocalTime.tm_sec,
          st_tsNow.tv_nsec / 1000000);
  fputs(szLogBuf,stdout);
}

namespace transfer{

//store match info for session match
map<string, MATCH_INFO> g_matchinfo_map;

template<typename T>
void safe_delete(T* ptr,const string key)
{
    LOG_DEBUG("destroy session begin.");
    if(key.empty())
    {
        return;
    }

    std::map<string,MATCH_INFO>::iterator iter  = g_matchinfo_map.find(key);
    if(iter != g_matchinfo_map.end())
    {
        g_matchinfo_map.erase(iter);
        if(ptr)
        {
            delete ptr;
            ptr = NULL;
        }
    }
    return;
    LOG_DEBUG("destroy session end.");
}

session::session(boost::asio::io_service& io_service): 
map_key_(""),
socket_(io_service),
is_reduplicate_(false),
response_str_(""),
session_closed_(false),
ismatched_checktimer_(io_service,boost::posix_time::seconds(TIMEOUT_S1)),
session_checktimer_(io_service,boost::posix_time::seconds(TIMEOUT_S2))
{
}

void session::session_close()
{
    LOG_DEBUG("session_close: begin close session "<<map_key_);
    if(!session_closed_)
    {
        delete_matchinfo();
        //this->socket_.cancel();
        ismatched_checktimer_.cancel();
        session_checktimer_.cancel();
        //socket_.shutdown(tcp::socket::shutdown_both);
        socket_.close();
        session_closed_ = true;
        LOG_INFO("session close....");
    }
    LOG_INFO("Session has been closed.");
}

session::~session()
{
    session_close();
}
/*******************************************************************************
 * Function Name: handle_check_ismatched
 * Description: check_ismatched timer handle function 
                check self-session if has been matched,if yes,transfer VNC
 * Auther:zhangliye 
 * Input : error
 * Output: 
 * Return: void
 *******************************************************************************/
void session::handle_check_ismatched(const boost::system::error_code& error)
{
    if (!error)
    {
        check_ismatched();
    } 
    else
    {
        LOG_ERROR("handle_check_ismatched error: "<<error.message().c_str());
        //if(error.value() != boost::system::errc::operation_canceled)
            session_close();
    } 
}

/*******************************************************************************
 * Function Name: handle_session_ismatched
 * Description: check_session_timer handle function 
                if self's life_counter >= TIMEOUT_COUNT,delete this
 * Auther:zhangliye 
 * Input : error
 * Output: 
 * Return: void
 *******************************************************************************/
void session::handle_check_session(const boost::system::error_code& error)
{
    LOG_DEBUG("handle_check_session: "<<map_key_<<" begin...");
    if (!error)
    {
        if(is_exist_in_map(map_key_))
        {
            MATCH_INFO matchinfo = g_matchinfo_map[map_key_];
            if(matchinfo.life_counter >= TIMEOUT_COUNT)
            {
                LOG_INFO("handle_check_session: check session, life_counter >= "<<TIMEOUT_COUNT<<", delete this session.");
                session_close();
                return;
            }
            restart_session_timer(TIMEOUT_S2);
        }
        else
        {
            LOG_ERROR("handle_check_session: map key is not exit. close session.");
            session_close();
            return;
        }
    } 
    else
    {
        LOG_ERROR("handle_check_session error: "<<error.message().c_str());
        //if(error.value() != boost::system::errc::operation_canceled)
            session_close();
    } 
}

/*******************************************************************************
 * Function Name: delete_matchinfo
 * Description:  
                if self's life_counter >= TIMEOUT_COUNT,delete self's matchinfo from map;
                if not,update life_counter 
 * Auther:zhangliye 
 * Input : 
 * Output: 
 * Return: void
 *******************************************************************************/
void session::delete_matchinfo()
{
    std::map<string,MATCH_INFO>::iterator iter_this  = g_matchinfo_map.find(map_key_);
    if(iter_this != g_matchinfo_map.end())
    {
        string session_id = iter_this->second.session_id;
        
        if(iter_this->second.is_matched)
        {
            //update the same session's other matchinfo's life_count = TIME_OUT
            //, when check session,delete matchinfo
            /*std::map<string,MATCH_INFO>::iterator iter  = g_matchinfo_map.begin();
            for(;iter != g_matchinfo_map.end();iter++)
            {
                LOG_INFO("map size = "<<g_matchinfo_map.size()<<": "<<iter->first.c_str()<<" is still in map.");
                MATCH_INFO matchinfo = iter->second;
                if(matchinfo.session_id == session_id && matchinfo.is_matched)
                {
                    LOG_INFO("update the same session "<<matchinfo.session_id.c_str()<<":"<<iter->first.c_str()<<"'s life_counter to "<<TIMEOUT_COUNT);
                    matchinfo.life_counter = TIMEOUT_COUNT;
                    g_matchinfo_map[iter->first] = matchinfo;
                    break;
                }
            }*/
            string dst_ip = iter_this->second.dst_ip;
            if(is_exist_in_map(dst_ip))
            {
                LOG_DEBUG("update the dst ip "<<dst_ip<<"'s life_counter to "<<TIMEOUT_COUNT);
                g_matchinfo_map[dst_ip].life_counter = TIMEOUT_COUNT;
            }
        }
        g_matchinfo_map.erase(iter_this);
        LOG_DEBUG("delete_matchinfo: map size = "<<g_matchinfo_map.size());
    }
}

/*******************************************************************************
 * Function Name: is_exist_in_map
 * Description: 
                if the specific key exists in map
 * Auther:zhangliye 
 * Input : key
 * Output: 
 * Return: exist,return true;not return false
 *******************************************************************************/
bool session::is_exist_in_map(const string& key)
{
    std::map<string,MATCH_INFO>::iterator iter  = g_matchinfo_map.find(key);
    if(iter != g_matchinfo_map.end())
    {
        return true;
    }
    return false;
}

/*******************************************************************************
 * Function Name: check_ismatched
 * Description: 
                check if self's matchinfo has been matched,if yes or no,write response to 
                client,otherwise update self's life_counter
 * Auther:zhangliye 
 * Input : 
 * Output: 
 * Return: 
 *******************************************************************************/
void session::check_ismatched()
{
    if(is_exist_in_map(map_key_))
    {
        std::map<string,MATCH_INFO>::iterator iter  = g_matchinfo_map.find(map_key_);
        if(iter->second.is_matched)
        {
            LOG_INFO("check_ismatched: matched, map_key_="<<map_key_.c_str());
            session_checktimer_.async_wait(boost::bind(&session::handle_check_session,
                        this,
                        boost::asio::placeholders::error));
            write_response(true, iter->second.header_maxcode, iter->second.header_mincode, "Session matched.");
            return;
        }
        else
        {    
            if(iter->second.life_counter >= TIMEOUT_COUNT)
            {
                write_response(false, iter->second.header_maxcode, iter->second.header_mincode, "Session quit,because time out while waiting to match info"); // hard code for test
                session_close();
                return;
            }
            iter->second.life_counter ++;
            LOG_INFO("check_ismatched: not matched, update life_count "<<iter->second.life_counter);
            restart_ismatched_timer(TIMEOUT_S1); 
        }
    }
    else
    {
        LOG_ERROR("check_ismatched: map_key_ is not exit in g_map.");
        session_close();
    }
    return;
}

/*******************************************************************************
 * Function Name: restart_session_timer
 * Description: 
                restart session timer
 * Auther: zhangliye 
 * Input : second 
 * Output: 
 * Return: void
 *******************************************************************************/
  void session::restart_session_timer(unsigned int second)
  {
    session_checktimer_.expires_from_now(boost::posix_time::seconds(second));
    session_checktimer_.async_wait(boost::bind(&session::handle_check_session,
                                            this,boost::asio::placeholders::error));
  }

/*******************************************************************************
 * Function Name: restart_ismatched_timer
 * Description: 
                restart ismatched timer
 * Auther: zhangliye 
 * Input : second 
 * Output: 
 * Return: void
 *******************************************************************************/
  void session::restart_ismatched_timer(unsigned int second)
  {
    ismatched_checktimer_.expires_from_now(boost::posix_time::seconds(second));
    ismatched_checktimer_.async_wait(boost::bind(&session::handle_check_ismatched,
                                            this,boost::asio::placeholders::error));
  }

  tcp::socket& session::socket()
  {
    return socket_;
  }

/*******************************************************************************
 * Function Name: start
 * Description: 
                socket session start communication 
 * Auther: zhangliye 
 * Input : 
 * Output: 
 * Return: void
 *******************************************************************************/
  void session::start()
  {
    boost::asio::socket_base::keep_alive option(true);
    socket_.set_option(option);
    read_matchmsg_header();
  }

/*******************************************************************************
 * Function Name: read_data
 * Description: 
                read transfer (VNC) data
 * Auther: zhangliye 
 * Input : 
 * Output: 
 * Return: void
 *******************************************************************************/
void session::read_data()
{
    memset(data_, 0, sizeof(data_));
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

/*******************************************************************************
 * Function Name: read_matchmsg_header
 * Description: 
                read CEMS_NET_HEAD
 * Auther: zhangliye 
 * Input : 
 * Output: 
 * Return: void
 *******************************************************************************/
void session::read_matchmsg_header()
{
    memset(header_, 0, sizeof(header_));
    socket_.async_read_some(boost::asio::buffer(header_,sizeof(CEMS_NET_HEAD)),
            boost::bind(&session::parse_matchmsg_header, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

/*******************************************************************************
 * Function Name: handle_read
 * Description: 
                transfer data read handle function
 * Auther: zhangliye 
 * Input :  
 * Output: error,bytes_transferred
 * Return: void
 *******************************************************************************/
void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        // print_timestamp();
        if(!is_exist_in_map(map_key_))
        {
            LOG_WARN("handle_read: map_key_ is not in g_map, close session.");
            session_close();
            return;
        }
        string dst_ip = g_matchinfo_map[map_key_].dst_ip;
        if(!is_exist_in_map(dst_ip))
        {
            LOG_WARN("handle_read: "<<dst_ip.c_str()<<" has been deleted, stop transfering, return.");
            session_close();
            return;
        }
        tcp::socket* dst_socketptr_ = g_matchinfo_map.at(dst_ip).socket_ptr;
        if(dst_socketptr_)
        {
            //LOG_INFO("handle_read: start wirte to "<<dst_ip.c_str());
            boost::asio::async_write(*dst_socketptr_, boost::asio::buffer(data_, bytes_transferred), boost::bind(&session::handle_write, this, boost::asio::placeholders::error));
        }
        else
        {
            LOG_ERROR("handle_read: dst socket is NULL. close session.");
            session_close();
        }
    }
    else
    {
        LOG_ERROR("handle_read error: "<< error.message().c_str());
        //if(error.value() != boost::system::errc::operation_canceled)
            session_close();
    }
}

/*******************************************************************************
 * Function Name: handle_write
 * Description: 
                transfer data write handle function
 * Auther: zhangliye 
 * Input : error
 * Output: 
 * Return: void
 *******************************************************************************/
void session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        // print_timestamp();
        read_data();
    }
    else
    {
        LOG_ERROR("handle_write error: "<<error.message().c_str());
        //if(error.value() != boost::system::errc::operation_canceled)
            session_close();
    }
}

/*******************************************************************************
 * Function Name: read_matchmsg_body
 * Description: 
                read matchinfo body 
 * Auther: zhangliye 
 * Input :  
        body_ptr: the point of body data
        body_len: the length of body data
 * Output: 
 * Return: void
 *******************************************************************************/
void session::read_matchmsg_body(char* body_ptr,int body_len)
{
    socket_.async_read_some(boost::asio::buffer(body_,BODY_LEN),
            boost::bind(&session::parse_matchmsg_body, this,
                body_ptr,body_len,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

/*******************************************************************************
 * Function Name: parse_matchmsg_header
 * Description: 
                parse the header of match info
 * Auther: zhangliye 
 * Input : 
          error: error info
          bytes_transferred: the size of transfer data bytes
 * Output: 
 * Return: void
 *******************************************************************************/
void session::parse_matchmsg_header(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        string remote_ip = this->socket_.remote_endpoint().address().to_string();
        map_key_ = remote_ip + ":" + boost::lexical_cast<string>(this) + ":" + boost::lexical_cast<string>(std::time(NULL));
        LOG_INFO("parse_matchmsg_header: map_key: "<<map_key_.c_str());

        msgheader msg_header(header_);
        PCEMS_NET_HEAD header = (PCEMS_NET_HEAD)header_;
        MATCH_INFO tmp;
        tmp.header_flag = header->dwFlag;
        tmp.header_version = header->dwVersion;
        tmp.header_msgcode = header->dwMsgCode;
        tmp.header_maxcode = header->dwMaxCode;
        tmp.header_mincode = header->dwMinCode;
        tmp.header_type = header->wType;
        tmp.header_count = header->wCount;
        tmp.header_index = header->wIndex;

        g_matchinfo_map.insert(std::pair<string,MATCH_INFO>(map_key_,tmp));
        ismatched_checktimer_.async_wait(boost::bind(&session::handle_check_ismatched, this, boost::asio::placeholders::error));
        LOG_INFO("parse_matchmsg_header: parse header, then intonsert data of key="<<map_key_.c_str()<<" into matchinfo map.");

        //DWORD body_len = msg_header.body_len();
        //char* body_ptr = new char[body_len]; //remember,change share_ptr ???
        //LOG_I("[%0x] parsed matchinfo header,json size = %d\n",this,body_len);
        //LOG_INFO("parse_matchmsg_header: parsed matchinfo header, json size="<<body_len);
        // charptr body_ptr(new char[body_len]);
        memset(body_, 0, sizeof(body_));
        read_matchmsg_body(body_, BODY_LEN);
    }
    else
    {
        //LOG_E("[%0x] parse_matchmsg_header error: %s\n",this,error.message().c_str());
        LOG_ERROR("parse_matchmsg_header error: "<<error.message().c_str());
        //if(error.value() != boost::system::errc::operation_canceled)
            session_close();
    }
}

/*******************************************************************************
 * Function Name: try_match
 * Description: 
                try to match info
 * Auther: zhangliye 
 * Input : 
          matchinfo: to be matched info
 * Output: 
 * Return: 
          true: match secceed
          fals: match fail
 *******************************************************************************/
bool session::try_match()
{
    if(map_key_.empty())
    {
        LOG_ERROR("try_match: map_key_ is empty.");
        return false;
    }

    if(!is_exist_in_map(map_key_))
    {
        LOG_ERROR("try_match: map_key_ is not in g_map");
        session_close();
        return false;
    }
    MATCH_INFO my_match_info = g_matchinfo_map[map_key_];

    map<string, MATCH_INFO>::iterator it;
    for(it = g_matchinfo_map.begin(); it != g_matchinfo_map.end(); ++it)
    {
        if(it->second.is_matched)
        {
            continue;
        }

        if(it->second.session_id == my_match_info.session_id 
           && it->second.src_device_id == my_match_info.dst_device_id
           && it->second.dst_device_id == my_match_info.src_device_id
           && it->second.header_mincode == my_match_info.header_mincode
           && it->second.device_type != my_match_info.device_type)
        {
            it->second.is_matched = true;
            it->second.dst_ip = map_key_;
            it->second.life_counter = 0;
            g_matchinfo_map[map_key_].is_matched = true;
            g_matchinfo_map[map_key_].dst_ip = it->first;
            g_matchinfo_map[map_key_].life_counter = 0;
            LOG_INFO("try_match: is matched! session_id="<<it->second.session_id<<", dst_device_id="<<it->second.dst_device_id<<", src_device_id="<<it->second.src_device_id<<", header_mincode="<<it->second.header_mincode);
            return true;
        }
    }
    return false;
}

#if 0
bool session::try_match(MATCH_INFO matchinfo)
{
    bool result = false;
    string secondsessionid = matchinfo.session_id;
    string secondsrcid = matchinfo.src_device_id;
    string seconddstid = matchinfo.dst_device_id;
    unsigned int secondmincode = matchinfo.header_mincode;
    string seconddevtype = matchinfo.device_type;
    /*LOG_I("[%0x] session:%s,src_id:%s,mincode:%d,dst_id:%s,dev_type:%s\n",this,secondsessionid.c_str()
      ,secondsrcid.c_str(),secondmincode,seconddstid.c_str(),seconddevtype.c_str());*/
    LOG_INFO("try_match: session:"<<secondsessionid.c_str()<<", src_id:"<<secondsrcid.c_str()<<", mincode:"<<secondmincode<<", dst_id:"<<seconddstid.c_str()<<", dev_type:"<<seconddevtype.c_str());

    map<string,MATCH_INFO>::iterator iter = g_matchinfo_map.begin();
    for(;iter != g_matchinfo_map.end();iter++)
    {
        string firstsessionid = iter->second.session_id;
        string firstsrcid = iter->second.src_device_id;
        unsigned int firstmincode = iter->second.header_mincode;
        string firstdstid = iter->second.dst_device_id;
        string firstdevtype = iter->second.device_type;

        if( !(iter->second.is_matched) 
                && matchinfo.session_id == iter->second.session_id
                && matchinfo.header_mincode == iter->second.header_mincode
                && matchinfo.src_device_id == iter->second.dst_device_id
                && matchinfo.dst_device_id == iter->second.src_device_id
                && matchinfo.device_type != iter->second.device_type)
        {
            //LOG_I("[%0x] session: %s %s \n",this,secondsessionid.c_str(),firstsessionid.c_str()); 
            //LOG_I("[%0x] src_id: %s %s \n",this,secondsrcid.c_str(),firstsrcid.c_str()); 
            //LOG_I("[%0x] mincode: %d %d \n",this,secondmincode,firstmincode); 
            //LOG_I("[%0x] dst_id: %s %s \n",this,seconddstid.c_str(),firstdstid.c_str()); 
            //LOG_I("[%0x] dev_type: %s %s \n",this,seconddevtype.c_str(),firstdevtype.c_str()); 
            //LOG_I("[%0x] match successfully,then update matchinfo\n",this);
            LOG_INFO("session: "<<secondsessionid.c_str()<<" "<<firstsessionid.c_str());
            LOG_INFO("src_id: "<<secondsrcid.c_str()<<" "<<firstsrcid.c_str());
            LOG_INFO("mincode: "<<secondmincode<<" "<<firstmincode);
            LOG_INFO("dst_id: "<<seconddstid.c_str()<<" "<<firstdstid.c_str());
            LOG_INFO("dev_type: "<<seconddevtype.c_str()<<" "<<firstdevtype.c_str());
            LOG_INFO("match successfully, then update matchinfo.");
            MATCH_INFO tmp;
            tmp = iter->second;
            tmp.life_counter = 0;
            tmp.is_matched = true;
            tmp.dst_ip = map_key_;// matchinfo.ip + boost::lexical_cast<string>(&socket_);
            g_matchinfo_map[iter->first] = tmp;

            matchinfo.life_counter = 0;
            matchinfo.is_matched = true;
            matchinfo.dst_ip = iter->first;//iter->second.ip + boost::lexical_cast<string>(iter->second.socket_ptr); 

            /*LOG_I("[%0x] matched: %s, %s\n",\
              this,map_key_.c_str(),matchinfo.dst_ip.c_str());*/
            LOG_INFO("matched: "<<map_key_.c_str()<<", "<<matchinfo.dst_ip.c_str());
            result = true;
            break;
        }
    }
    g_matchinfo_map[map_key_] = matchinfo; 
    // g_matchinfo_map.insert(std::pair<string,MATCH_INFO>(matchinfo.ip,matchinfo));
    //LOG_I("[%0x] parsed body,then update data of key=%s into matchinfo map\n",this,matchinfo.ip.c_str());
    LOG_INFO("parsed body, then update data of key="<<matchinfo.ip.c_str()<<" into matchinfo map.");
    return result;
}
#endif

/*******************************************************************************
 * Function Name: write_response
 * Description: 
                write result of match info to client
 * Auther: zhangliye 
 * Input : 
          result,maxcode,mincode,description
 * Output: 
 * Return: void
 *******************************************************************************/
void session::write_response(bool result, unsigned int maxcode, unsigned int mincode, string description)
{
    //ismatched_checktimer_.cancel();//time has job done,cancel it
    if(!is_exist_in_map(map_key_))
    {
        session_close();
        return;
    }

    msgresponse msg_response;
    if(result)
    {
        msg_response.init_response(mincode, 0, description, maxcode);
        response_str_ = msg_response.response();
        LOG_INFO("write_response: write to "<<map_key_.c_str()<<" matched info:"<<response_str_.c_str());
    }
    else
    {
        msg_response.init_response(mincode, 1, description, maxcode);
        response_str_ = msg_response.response();
        LOG_INFO("write_response: write to "<<map_key_.c_str()<<" not matched info:"<<response_str_.c_str());
        session_close();
        return;
    }

    MATCH_INFO tmp = g_matchinfo_map.at(map_key_);
    CEMS_NET_HEAD header;
    header.dwDataSize = response_str_.size();
    header.dwFlag = tmp.header_flag;
    header.dwVersion = tmp.header_version;
    header.dwMsgCode = tmp.header_msgcode;
    header.dwMaxCode = tmp.header_maxcode;
    header.dwMinCode = tmp.header_mincode;
    header.wHeadSize = sizeof(CEMS_NET_HEAD);
    header.wType = tmp.header_type;
    header.wCount = tmp.header_count;
    header.wIndex = tmp.header_index;

    LOG_DEBUG("write_response: header.dwDataSize="<<header.dwDataSize);
    
    boost::asio::async_write(socket_, boost::asio::buffer(&header, sizeof(CEMS_NET_HEAD)), boost::bind(&session::handle_write_header, this, boost::asio::placeholders::error));
}

/*******************************************************************************
 * Function Name: handle_write_header
 * Description: 
                write header of response info
 * Auther: zhangliye 
 * Input : 
          error
 * Output: 
 * Return: void
 *******************************************************************************/
void session::handle_write_header(const boost::system::error_code& error)
{
    if(!error)
    {
        char* str_ptr = (char*)response_str_.c_str();

        boost::asio::async_write(socket_, boost::asio::buffer(str_ptr,response_str_.size()), boost::bind(&session::handle_write_body, this, boost::asio::placeholders::error));
    }
    else
    {
        LOG_ERROR("handle_write_header error: "<<error.message().c_str());
        //if(error.value() != boost::system::errc::operation_canceled)
            session_close();
    }
}

/*******************************************************************************
 * Function Name: handle_write_body
 * Description: 
                write body of response info
 * Auther: zhangliye 
 * Input : 
          error
 * Output: 
 * Return: void
 *******************************************************************************/
void session::handle_write_body(const boost::system::error_code& error)
{
    if (!error)
    {
        if(!is_exist_in_map(map_key_))
        {
            LOG_WARN("handle_write_body: map_key_ is not exit in g_map.");
            session_close();
            return;
        }
        if(g_matchinfo_map.at(map_key_).is_matched)
        {
            LOG_INFO("handle_write_body: start session check timer.");
/*            session_checktimer_.async_wait(boost::bind(&session::handle_check_session,
                        this,
                        boost::asio::placeholders::error));*/
            read_data();
        }
        else
        {
            LOG_ERROR("handle_write_body: not matched, close this session.");
            session_close();
        }
    }
    else
    {
        //LOG_E("[%0x] handle_write_body error: %s\n",this,error.message().c_str());
        LOG_ERROR("handle_write_body error: "<<error.message().c_str());
        //if(error.value() != boost::system::errc::operation_canceled)
            session_close();
    }
}

/*******************************************************************************
 * Function Name: is_exist_in_map
 * Description: 
                check if the specific matchinfo exists in map
 * Auther: zhangliye 
 * Input : 
          matchinfo
 * Output: 
 * Return: 
          true: exist
          false: not exist
 *******************************************************************************/
#if 0
bool session::is_exist_in_map(const MATCH_INFO &matchinfo)
{
    bool result = false;
    map<string,MATCH_INFO>::iterator iter = g_matchinfo_map.begin();
    for(;iter != g_matchinfo_map.end();iter++)
    {

        if( !(iter->second.is_matched) 
                && matchinfo.header_mincode == iter->second.header_mincode
                && matchinfo.dst_device_id == iter->second.dst_device_id
                && matchinfo.src_device_id == iter->second.src_device_id
                && matchinfo.device_type == iter->second.device_type)
        {
            //LOG_E("[%0x] same mathinfo has existed in map\n",this);
            /*LOG_E("[%0x] src_id:%s,dst_id:%s,mincode:%d,dev_type:%s\n",this
              ,matchinfo.src_device_id.c_str()
              ,matchinfo.dst_device_id.c_str()
              ,matchinfo.header_mincode
              ,matchinfo.device_type.c_str()); */

            LOG_ERROR("is_exist_in_map: same mathinfo has existed in map.");
            LOG_ERROR("is_exist_in_map: src_id:"<<matchinfo.src_device_id.c_str()<<", dst_id:"<<matchinfo.dst_device_id.c_str()<<", mincode:"<<matchinfo.header_mincode<<", dev_type:"<<matchinfo.device_type.c_str());

            result = true;
            is_reduplicate_ = true;
        } 
    }
    return result; 
}
#endif

/*******************************************************************************
 * Function Name: parse_matchmsg_body
 * Description: 
                parse the body of matchinfo
 * Auther: zhangliye 
 * Input : 
          body_ptr: the point of matchinfo's body data
          body_len: the size of matchinfo's body
          error:
 * Output: 
 * Return: 
 *******************************************************************************/
void session::parse_matchmsg_body(char* body_ptr,int body_len,const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        if(body_ptr != NULL)
        {
            LOG_DEBUG("parse_matchmsg_body: message body["<<body_ptr<<"].");
        }
        else
        {
            LOG_ERROR("parse_matchmsg_body: message is NULL.");
            //delete_matchinfo();
            session_close();
            return;
        }
        msgbody msg_body(body_ptr,body_len);
        if(msg_body.parse_body() < 0)
        {
            LOG_ERROR("parse_matchmsg_body: parse body error.");
            //delete_matchinfo();
            session_close();
            return;
        }
        g_matchinfo_map[map_key_].ip = socket_.remote_endpoint().address().to_string();
        g_matchinfo_map[map_key_].socket_ptr = &socket_;
        g_matchinfo_map[map_key_].session_id = msg_body.get_sessionid();
        g_matchinfo_map[map_key_].src_device_id = msg_body.get_srcdeviceid();
        g_matchinfo_map[map_key_].dst_device_id = msg_body.get_dstdeviceid();
        g_matchinfo_map[map_key_].device_type = msg_body.get_devicetype();

        try_match(); 
    }
    else
    {
        LOG_ERROR("parse_matchmsg_body error: "<<error.message().c_str());
        // this->ismatched_checktimer_.cancel();
        //if(error.value() != boost::system::errc::operation_canceled)
            session_close();
    }
}

  server::server(boost::asio::io_service& io_service, short port)
    : io_service_(io_service)
    ,acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
  {
    //LOG_I("listening %d\n",port);
    LOG_INFO("listening "<<port);
    start_accept();
  }

  void server::start_accept()
  {
    session* new_session = new session(io_service_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }

  void server::handle_accept(session* new_session,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      //LOG_I("[%0x] accepted a new session\n",new_session);
      LOG_INFO("handle_accept: accepted a new Session.");
      new_session->start();
    }
    else
    {
      //LOG_E("handle_accept error,delete this session\n");
      LOG_ERROR("handle_accept error, delete this session.");
      delete new_session;
    }

    start_accept();
  }
}
