
namespace java com.vrv.cems.service.base.interfaces
namespace cpp com.vrv.cems.common.thrift.service

service CommonService
{
    binary getDataTS(1:string maxCode,2:string minCode,3:string checkCode,4:bool isZip,5:binary data, 6:bool isEncrypt, 7:string key, 8:i32 flag)
	
    binary getDataTC(1:string maxCode,2:string minCode,3:string checkCode,4:bool isZip,5:binary data,6:string sessionId,7:i32 msgCode)
}
