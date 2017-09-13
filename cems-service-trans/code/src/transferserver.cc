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


namespace transfer
{
  //store match info for session match
  map<string,MATCH_INFO> g_matchinfo_map;

  template<typename T>
  void safe_delete(T* ptr,const string key)
  {
    std::map<string,MATCH_INFO>::iterator iter  = g_matchinfo_map.find(key);
    if(iter != g_matchinfo_map.end())
    {
      if(ptr)
      {
        delete ptr;
        ptr = NULL;
      }
    }
    /*else                        //以后再发现句柄泄漏可以从这里入手修改
    {
	  if(key.empty())
	  {
		  delete ptr; 
		  ptr = NULL;
	  }
    }*/
  }

  session::session(boost::asio::io_service& io_service)
    : socket_(io_service),is_reduplicate_(false)
    ,ismatched_checktimer_(io_service,boost::posix_time::seconds(TIMEOUT_S1))
    ,session_checktimer_(io_service,boost::posix_time::seconds(TIMEOUT_S2))
  {
    this->header_ = new char[sizeof(CEMS_NET_HEAD)];
  }


  session::~session()
  {
    //this->socket_.cancel();
    this->socket_.shutdown(tcp::socket::shutdown_both);
    this->socket_.close();
    this->ismatched_checktimer_.cancel();
    this->session_checktimer_.cancel();
    delete_matchinfo();
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
      ////LOG_E("[%0x] handle_check_ismatched error: %s\n",this,error.message().c_str());
      LOG_ERROR("handle_check_ismatched error: "<<error.message().c_str());
      if(error.value() != boost::system::errc::operation_canceled)
        safe_delete(this,map_key_);  
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
    if (!error)
    {
      if(is_exist_in_map(map_key_))
      {
        MATCH_INFO matchinfo = g_matchinfo_map.at(this->map_key_);
        if(matchinfo.life_counter >= TIMEOUT_COUNT)
        {
          //LOG_I("[%0x] check session,life_counter >= %d,delete this session\n",this,TIMEOUT_COUNT);
          LOG_INFO("handle_check_session: check session, life_counter >= "<<TIMEOUT_COUNT<<", delete this session.");
          // session_checktimer_.cancel();
          safe_delete(this,map_key_);
          return;
        }
      }
      restart_session_timer(TIMEOUT_S2);
    } 
    else
    {
      //LOG_E("[%0x] handle_check_session error: %s\n",this,error.message().c_str());
      LOG_ERROR("handle_check_session error: "<<error.message().c_str());
      if(error.value() != boost::system::errc::operation_canceled)
        safe_delete(this,map_key_);
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
      //LOG_D("[%0x] erase %s from map and delete this session\n",this,map_key_.c_str());
      LOG_DEBUG("delete_matchinfo: erase "<<map_key_.c_str()<<" from map and delete this session.");
      g_matchinfo_map.erase(iter_this);
//      LOG_D("[%0x] map size = %d\n",this,g_matchinfo_map.size());
      LOG_DEBUG("delete_matchinfo: map size = "<<g_matchinfo_map.size());
      if(!is_reduplicate_)
      {
        //update the same session's other matchinfo's life_count = TIME_OUT
        //, when check session,delete matchinfo
        std::map<string,MATCH_INFO>::iterator iter  = g_matchinfo_map.begin();
        for(;iter != g_matchinfo_map.end();iter++)
        {
          //LOG_I("[%0x] map size = %d: %s is still in map\n",this,g_matchinfo_map.size(),iter->first.c_str()); 
          LOG_INFO("map size = "<<g_matchinfo_map.size()<<": "<<iter->first.c_str()<<" is still in map.");
          MATCH_INFO matchinfo = iter->second;
          if(matchinfo.life_counter < TIMEOUT_COUNT
            && matchinfo.session_id == session_id
            && matchinfo.is_matched)
          {
            /*LOG_I("[%0x] update the same session %s:%s's life_counter to %d\n",
                              this,matchinfo.session_id.c_str(),iter->first.c_str(),TIMEOUT_COUNT);*/
              LOG_INFO("update the same session "<<matchinfo.session_id.c_str()<<":"<<iter->first.c_str()<<"'s life_counter to "<<TIMEOUT_COUNT);
            matchinfo.life_counter = TIMEOUT_COUNT;
            g_matchinfo_map[iter->first] = matchinfo;
          }
        }
      }
    }
  }

/*******************************************************************************
 * Function Name: is_existed_in_map
 * Description: 
                if the specific key exists in map
 * Auther:zhangliye 
 * Input : key
 * Output: 
 * Return: exist,return true;not return false
 *******************************************************************************/
  bool session::is_exist_in_map(const string key)
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
      MATCH_INFO matchinfo = g_matchinfo_map.at(this->map_key_);
      if(matchinfo.is_matched)
      {
        //LOG_I("[%0x] matched,write response to %s\n",this,map_key_.c_str());
        LOG_INFO("check_ismatched: matched, write response to "<<map_key_.c_str());
        write_response(true,matchinfo.header_maxcode,matchinfo.header_mincode,""); // hard code for test
      }
      else
      {
         std::map<string,MATCH_INFO>::iterator iter  = g_matchinfo_map.find(map_key_);
        if(matchinfo.session_id.size())
        {
          MATCH_INFO tmp = iter->second;
          tmp.life_counter++;
          //LOG_I("[%0x] not matched,update life_counter %d\n",this,tmp.life_counter);
          LOG_INFO("check_ismatched: not matched, update life_count "<<tmp.life_counter);
          if(tmp.life_counter >= TIMEOUT_COUNT)
          {
            write_response(false,matchinfo.header_maxcode,matchinfo.header_mincode,"Session quit,because time out while waiting to match info"); // hard code for test
          }
          else
          {
            g_matchinfo_map[iter->first] = tmp; 
            restart_ismatched_timer(TIMEOUT_S1); 
          }
        }
        else
        {
          //LOG_E("[%0x] session id is empty\n",this);
          LOG_ERROR("check_ismatched: session id is empty.");
          restart_ismatched_timer(TIMEOUT_S1); 
        }
      }
    }
    else
    {
      restart_ismatched_timer(TIMEOUT_S1);
    }
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
  void session::handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
    // printf("[%0x] start rev:",this);
    // print_timestamp();
        string dst_ip = g_matchinfo_map.at(this->map_key_).dst_ip;
        if(!is_exist_in_map(dst_ip))
        {
          //LOG_I("%s has been deleted,stop transfering,return\n",dst_ip.c_str());
          LOG_INFO("handle_read: "<<dst_ip.c_str()<<" has been deleted, stop transfering, return.");
          return;
        }
        tcp::socket* dst_socketptr_ = g_matchinfo_map.at(dst_ip).socket_ptr;
        if(dst_socketptr_)
        {
          //LOG_I("start write to %s\n",dst_ip.c_str());
          LOG_INFO("handle_read: start wirte to "<<dst_ip.c_str());
          boost::asio::async_write(*dst_socketptr_,
              boost::asio::buffer(data_, bytes_transferred),
              boost::bind(&session::handle_write, this,
                boost::asio::placeholders::error));
        }
    }
    else
    {
      //LOG_E("[%0x] handle_read error: %s\n",this,error.message().c_str());
      LOG_ERROR("handle_read error: "<< error.message().c_str());
      if(error.value() != boost::system::errc::operation_canceled)
        safe_delete(this,map_key_);
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
      // printf("[%0x] end wrt:",this);
      // print_timestamp();
      read_data();
    }
    else
    {
      //LOG_E("[%0x] handle_write error: %s\n",this,error.message().c_str());
      LOG_ERROR("handle_write error: "<<error.message().c_str());
      // session_checktimer_.cancel();
      if(error.value() != boost::system::errc::operation_canceled)
        safe_delete(this,map_key_);
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
    socket_.async_read_some(boost::asio::buffer(body_ptr,body_len),
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
  void session::parse_matchmsg_header(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      ismatched_checktimer_.async_wait(boost::bind(&session::handle_check_ismatched,
                                          this,
                                          boost::asio::placeholders::error));

      string remote_ip = this->socket_.remote_endpoint().address().to_string();
      map_key_ = remote_ip + ":" + boost::lexical_cast<string>(this) + ":" + boost::lexical_cast<string>(std::time(NULL));
      //LOG_I("[%0x] map_key: %s\n",this,map_key_.c_str());
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
      //LOG_I("[%0x] parse header,then intonsert data of key=%s into matchinfo map\n",this,map_key_.c_str());
      LOG_INFO("parse_matchmsg_header: parse header, then intonsert data of key="<<map_key_.c_str()<<" into matchinfo map.");

      DWORD body_len = msg_header.body_len();
      char* body_ptr = new char[body_len]; //remember,change share_ptr ???
      //LOG_I("[%0x] parsed matchinfo header,json size = %d\n",this,body_len);
      LOG_INFO("parse_matchmsg_header: parsed matchinfo header, json size="<<body_len);
      // charptr body_ptr(new char[body_len]);
      read_matchmsg_body(body_ptr,body_len);
    }
    else
    {
      //LOG_E("[%0x] parse_matchmsg_header error: %s\n",this,error.message().c_str());
      LOG_ERROR("parse_matchmsg_header error: "<<error.message().c_str());
      // ismatched_checktimer_.cancel();
      if(error.value() != boost::system::errc::operation_canceled)
      {
        safe_delete(this,map_key_);
	if(map_key_.empty())
	{
	     delete this;
	}
      }
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
  void session::write_response(bool result,unsigned int maxcode,unsigned int mincode,string description)
  {
    // ismatched_checktimer_.cancel();//time has job done,cancel it
    if(!is_exist_in_map(map_key_))
      return;  

    msgresponse msg_response;
    if(is_exist_in_map(map_key_) 
      && g_matchinfo_map.at(this->map_key_).is_matched)
    {
      msg_response.init_response(mincode,0,description,maxcode);
      response_str_ = msg_response.response();
      //LOG_I("[%0x] wirte to %s matched info:%s\n",this,map_key_.c_str(),response_str_.c_str());
      LOG_INFO("write_response: write to "<<map_key_.c_str()<<" matched info:"<<response_str_.c_str());
    }
    else
    {
      msg_response.init_response(mincode,1,description,maxcode);
      response_str_ = msg_response.response();
      //LOG_I("[%0x] wirte to %s not matched info:%s\n",this,map_key_.c_str(),response_str_.c_str());
      LOG_INFO("write_response: write to "<<map_key_.c_str()<<" not matched info:"<<response_str_.c_str());
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

    //LOG_D("[%0x]header.dwDataSize = %d\n",this,header.dwDataSize);
    LOG_DEBUG("write_response: header.dwDataSize="<<header.dwDataSize);

    boost::asio::async_write(socket_,
      boost::asio::buffer(&header,sizeof(CEMS_NET_HEAD)),
      boost::bind(&session::handle_write_header, this,
        boost::asio::placeholders::error));
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

      boost::asio::async_write(socket_,
        boost::asio::buffer(str_ptr,response_str_.size()),
        boost::bind(&session::handle_write_body, this,
          boost::asio::placeholders::error));
    }
    else
    {
      //LOG_E("[%0x] handle_write_header error: %s\n",this,error.message().c_str());
      LOG_ERROR("handle_write_header error: "<<error.message().c_str());
      // ismatched_checktimer_.cancel();
      if(error.value() != boost::system::errc::operation_canceled)
        safe_delete(this,map_key_);
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
      // if is matched = true false ???
      if(g_matchinfo_map.at(map_key_).is_matched)
      {
        //LOG_I("[%0x] start session check timer\n",this);
        LOG_INFO("handle_write_body: start session check timer.");
        session_checktimer_.async_wait(boost::bind(&session::handle_check_session,
                                        this,
                                          boost::asio::placeholders::error));
        read_data();
      }
      else
      {
        //LOG_E("[%0x] not matched,delete this session\n",this);
        LOG_ERROR("handle_write_body: not matched, delete this session.");
        safe_delete(this,map_key_);
      }
        // session_checktimer_.cancel();
        // safe_delete(this,map_key_);
    }
    else
    {
      //LOG_E("[%0x] handle_write_body error: %s\n",this,error.message().c_str());
      LOG_ERROR("handle_write_body error: "<<error.message().c_str());
      // session_checktimer_.cancel();
      if(error.value() != boost::system::errc::operation_canceled)
        safe_delete(this,map_key_);
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
  void session::parse_matchmsg_body(char* body_ptr,int body_len,const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      msgbody msg_body(body_ptr,body_len);
      MATCH_INFO matchinfo = g_matchinfo_map.at(map_key_);
      matchinfo.ip = this->socket_.remote_endpoint().address().to_string();
      matchinfo.socket_ptr = &socket_;
      matchinfo.session_id = msg_body.get_sessionid();
      matchinfo.src_device_id = msg_body.get_srcdeviceid();
      matchinfo.dst_device_id = msg_body.get_dstdeviceid();
      matchinfo.device_type = msg_body.get_devicetype();

      //check if this matchinfo has existed in map,refuse this connection
      if(!is_exist_in_map(matchinfo))
      {
        try_match(matchinfo);
      }
      else
      {
        //LOG_E("[%0x] matchinfo is replicated,delete self\n",this);
        LOG_ERROR("parse_matchmsg_body: matchinfo is replicated, delete self.");
        // this->socket_.close();
        // this->ismatched_checktimer_.cancel();
        safe_delete(this,map_key_);
      }
      delete body_ptr;
    }
    else
    {
      //LOG_E("[%0x] handle_write_body error: %s\n",this,error.message().c_str());
      LOG_ERROR("parse_matchmsg_body error: "<<error.message().c_str());
      // this->ismatched_checktimer_.cancel();
      if(error.value() != boost::system::errc::operation_canceled)
        safe_delete(this,map_key_);
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
