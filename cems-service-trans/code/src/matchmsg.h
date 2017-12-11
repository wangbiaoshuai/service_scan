#ifndef _MATCHMSG_H
#define _MATCHMSG_H

#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "common.h"
#include "json/json.h"
#include "json/value.h"

using namespace std;

/***
2050 input:
	maxCode=00011200
	minCode=2450
	checkCode=
	isZip=false
	sessionId=b5019b56795b49fe8ce1cd0d3477efe9
	msgCode=
	data={
	"SessionID":" ",  //会话ID, 该ID，在WEB平台生成，用于区分每次的远程协助
	"SrcDeviceID":" ",   //源终端ID
	"DstDeviceID":" ",   //目标终端ID
	"SystemType":"0"    //软件种类, 0：CEMS， 1：VRV内网，2：单机版
	"DeviceType ":" ",  //终端类型，0：主控端；1：被控端
	 }

2050 output:
	{
	  "maxCode":"00011200",
	  "minCode":"2450",
	  "result":"0",//0表示成功；非0表示失败
	  "description":""
	}


2051 input:
	maxCode=00011200
	minCode=2451
	checkCode=
	isZip=false
	sessionId=b5019b56795b49fe8ce1cd0d3477efe9
	msgCode=
	data={
	"SessionID":" ",  //会话ID, 该ID，在WEB平台生成，用于区分每次的远程协助
	"SrcDeviceID":" ",   //源终端ID
	"DstDeviceID":" ",   //目标终端ID
	"SystemType":"0"    //软件种类,0：CEMS， 1：VRV内网，2：单机版
	"DeviceType ":" ",  //终端类型，0：主控端；1：被控端
	 }

2050 output:
	{
	  "maxCode":"00011200",
	  "minCode":"2451",
	  "result":"0",//0表示成功；非0表示失败
	  "description":""
	 }
***/

namespace transfer
{
	class msgheader
	{
	public:
		msgheader(const char *data_ptr);
		unsigned int body_len();
	private:
		PCEMS_NET_HEAD header_;
	};

	class msgbody
	{
	public:
		msgbody(const char *data_ptr,unsigned int data_len);
        int parse_body();
		string get_sessionid();
		string get_srcdeviceid();
		string get_dstdeviceid();
		string get_devicetype();
		bool isMatched();

	private:
		string json_;
		string session_id;
	    	string src_device_id;
    		string dst_device_id;
	    	string device_type;
	};

	class msgresponse
	{
	public:
		msgresponse(){};
		void init_response(unsigned int mincode,unsigned short result,
					string description,unsigned short maxcode);
		string response();
		
	private:
		string maxcode_;
		string mincode_;
		string result_;
		string description_;
	};
}

#endif
