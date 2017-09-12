#include "matchmsg.h"

namespace transfer
{
	msgheader::msgheader(const char *data_ptr)
	{
		header_ = (PCEMS_NET_HEAD)data_ptr;
	}

	unsigned int msgheader::body_len()
	{
		return header_->dwDataSize;
	}

	msgbody::msgbody(const char *data_ptr,unsigned int data_len)
	:json_(data_ptr,data_len)
	{
		Json::Reader reader;  
		Json::Value root;  	
		if (reader.parse(json_, root)) 
    	{  
        	session_id = root["SessionID"].asString(); 
        	src_device_id = root["SrcDeviceID"].asString(); 
        	dst_device_id = root["DstDeviceID"].asString(); 
        	device_type = root["DeviceType"].asString(); 
    	}  
	}

	string msgbody::get_sessionid()
	{
		return session_id;
	}

	string msgbody::get_srcdeviceid()
	{
		return src_device_id;
	}

	string msgbody::get_dstdeviceid()
	{
		return dst_device_id;
	}

	string msgbody::get_devicetype()
	{
		return device_type;
	}

	bool msgbody::isMatched()
	{
		string qqq;
		return true;
	}

	void msgresponse::init_response(unsigned int mincode,unsigned short result,
					string description,unsigned short maxcode)
	{
		mincode_ = boost::lexical_cast<string>(mincode);
		result_  = boost::lexical_cast<string>(result);
		description_ = description;
		maxcode_ = boost::lexical_cast<string>(maxcode);
	}

	string msgresponse::response()
	{
		string result = "{ \"maxCode\":\"" + maxcode_ + "\",\"minCode\":\"" + mincode_ + "\",\"result\":\" " + result_ + "\",\"description\":\"" + description_ + "\"}";
		return result;
	}
}
