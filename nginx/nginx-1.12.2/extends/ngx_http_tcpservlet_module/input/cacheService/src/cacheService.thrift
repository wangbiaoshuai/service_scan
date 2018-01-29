/**
 * <B>说 明</B>:缓存服务Thrift文件-接口
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
include "bean.thrift"
namespace java com.vrv.cems.service.base.interfaces
namespace cpp com.vrv.cems.service.base.interfaces
 
 /**
 * <B>说 明</B>:缓存服务接口
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 service CacheService{
 
  /**
	 * 非客户端数据通讯标准接口
	 * @param maxCode		主功能号
	 * @param minCode		子功能号
	 * @param checkCode data数据crc校验值
	 * @param isZip     data数据是否压缩：true压缩;false不压缩
	 * @param data      data通讯的业务数据
	 * @param isEncrypt data数据是否加密：true加密;false不加密
	 * @param key       UUID
	 * @param flag      为计算密钥生成的随机数
	 */
    binary getDataTS(1:string maxCode,2:string minCode,3:string checkCode,4:bool isZip,5:binary data, 6:bool isEncrypt, 7:string key, 8:i32 flag)
 
	/**
	 * 验证Key在缓存中是否已存在
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1
	 * @param prefix		前缀
	 * @param key 			需要验证的Key
	 * @return
	 *		Result(0,存在.);
	 *		Result(1,不存在.);
	 *		Result(2,验证信息参数不正确[前缀prifix为空]!);
	 *		Result(3,验证信息参数不正确[key为空]!);
	 */
	bean.Result isExist(1:string maxCode,2:string minCode,3:string prefix,4:string key)
		
	/**
	 * 设置缓存数据有效期限
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：2
	 * @param key 			需要缓存数据的Key
	 * @param expireTime    有效期，单位是秒
	 * @return
	 *		Result(0,保存有有效期的数据成功);
	 *		Result(1,保存有有效期的数据失败[expireTime为空]!);
	 */
	bean.Result setExpireTimeByString(1:string maxCode,2:string minCode,3:string key,4:i32 expireTime)
	
	/**
	 * 设置缓存数据有效期限
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：3
	 * @param key 			需要缓存数据的Key
	 * @param expireTime 	有效期，单位是秒
	 * @return
	 *		Result(0,保存有有效期的数据成功);
	 *		Result(1,保存有有效期的数据失败[expireTime为空]!);
	 */
	bean.Result setExpireTimeByBinary(1:string maxCode,2:string minCode,3:binary key,4:i32 expireTime)
	
	/**
	 * 批量删除缓存信息
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：100
	 * @param keyList			    需要删除的缓存信息的键的集合
	 * @return	
	 *		Result(0,删除缓存信息成功.);
	 *		Result(1,删除缓存信息失败[传入的键集合为空]!);
	 */
	bean.Result batchDel(1:string maxCode,2:string minCode,3:list<string> keyList)
	
	 /**
	 * 保存设备信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：101
	 * @param deviceCache	需要保存的设备信息
	 * @return
	 *		Result(0,保存设备信息到缓存成功.);
	 *		Result(1,保存设备信息到缓存失败[设备信息为空]!);
	 *		Result(2,保存设备信息到缓存失败[设备devOnlyId为空]!);
	 */
	bean.Result saveDevice(1:string maxCode,2:string minCode,3:bean.DeviceCache deviceCache)
	
	/**
	 * 更新设备信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：102
	 * @param deviceCache	需要更新的设备信息
	 * @return	
	 *		Result(0,更新设备信息到缓存成功.);
	 *		Result(1,更新设备信息到缓存失败[设备信息为空]!);
	 *		Result(2,更新设备信息到缓存失败[设备devOnlyId为空]!);
	 */
	bean.Result updateDevice(1:string maxCode,2:string minCode,3:bean.DeviceCache deviceCache)
	
	/**
	 * 更新设备信息,只更新指定属性信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：103
	 * @param devOnlyId			设备唯一ID
	 * @param fieldValueMap		需要更新的属性名及属性值
	 * @return	
	 *		Result(0,更新设备信息到缓存成功.);
	 *		Result(1,更新设备信息到缓存失败[需要更新的属性名及属性值为空]!);
	 *		Result(2,更新设备信息到缓存失败[设备devOnlyId为空]!);
	 */
	bean.Result updateDeviceByField(1:string maxCode,2:string minCode,3:string devOnlyId,4:map<string,string> fieldValueMap )
	
	/**
	 * 查询设备信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：104
	 * @param deviceCache		需要查询的设备信息(至少devOnlyId不能为空)
	 * @return	
	 *		DeviceCache	查询到的设备信息
	 */
	bean.DeviceCache queryDevice(1:string maxCode,2:string minCode,3:bean.DeviceCache deviceCache)
	
	/**
	 * 查询设备信息,根据devOnlyId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：105
	 * @param devOnlyId			设备唯一ID
	 * @return	
	 *		DeviceCache	查询到的设备信息
	 */
	bean.DeviceCache queryDeviceByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 查询设备信息,根据IP 
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：106
	 * @param ip				设备IP地址
	 * @return	
	 *		DeviceCache		查询到的设备信息
	 */
	bean.DeviceCache queryDeviceByIp(1:string maxCode,2:string minCode,3:string ip)
	
	/**
	 * 删除设备信息 
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：107
	 * @param deviceCache		需要删除的设备信息(至少devOnlyId不能为空)
	 * @return	
	 *		Result(0,从缓存中删除设备信息成功.);
	 *		Result(1,从缓存中删除设备信息失败[需要删除的设备信息输入值为空]!);
	 *		Result(2,从缓存中删除设备信息失败[需要删除的设备信息输入值devOnlyId为空]!);
	 */
	bean.Result deleteDevice(1:string maxCode,2:string minCode,3:bean.DeviceCache deviceCache)
	
	/**
	 * 删除设备信息,根据devOnlyId 
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：108
	 * @param devOnlyId			devOnlyId
	 * @return	
	 *		Result(0,从缓存中删除设备信息成功.);
	 *		Result(1,从缓存中删除设备信息失败[需要删除的设备信息输入值devOnlyId为空]!);
	 */
	bean.Result deleteDeviceByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 删除设备信息,根据IP 
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：109
	 * @param ip				IP地址
	 * @return	
	 *		Result(0,从缓存中删除设备信息成功.);
	 *		Result(1,从缓存中删除设备信息失败[需要删除的设备信息输入值ip为空]!);
	 */
	bean.Result deleteDeviceByIp(1:string maxCode,2:string minCode,3:string ip)
	
	/**
	 * 批量保存设备信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：110
	 * @param deviceCacheList	需要保存的设备信息集合
	 * @return
	 *		Result(0,保存设备信息到缓存成功.);
	 *		Result(1,保存设备信息到缓存失败[设备信息为空]!);
	 *		Result(2,保存设备信息到缓存失败[设备devOnlyId为空]!);
	 */
	list<bean.Result> batchSaveDevice(1:string maxCode,2:string minCode,3:list<bean.DeviceCache> deviceCacheList)
	
	/**
	 * 批量更新设备信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：111
	 * @param deviceCacheList	需要保存的设备信息集合
	 * @return
	 *		Result(0,更新设备信息到缓存成功.);
	 *		Result(1,更新设备信息到缓存失败[需要更新的属性名及属性值为空]!);
	 *		Result(2,更新设备信息到缓存失败[设备devOnlyId为空]!);
	 */
	list<bean.Result> batchUpdateDevice(1:string maxCode,2:string minCode,3:list<bean.DeviceCache> deviceCacheList)
	
	/**
	 * 批量查询设备信息
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：112
	 * @param deviceCacheList		需要查询的设备信息集合(至少devOnlyId不能为空)
	 * @return	
	 *		list<DeviceCache>	查询到的设备信息
	 */
	list<bean.DeviceCache> batchQueryDevice(1:string maxCode,2:string minCode,3:list<bean.DeviceCache> deviceCacheList)
	
	/**
	 * 批量查询设备信息，根据设备devOnlyId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：113
	 * @param deviceCache		需要查询的设备信息(至少devOnlyId不能为空)
	 * @return	
	 *		list<DeviceCache>	查询到的设备信息集合
	 */
	list<bean.DeviceCache> batchQueryDeviceByDevOnlyId(1:string maxCode,2:string minCode,3:list<string> devOnlyIdList)
	
	/**
	 * 批量删除设备信息 
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：114
	 * @param deviceCacheList		需要删除的设备信息集合(至少devOnlyId不能为空)
	 * @return	
	 *		Result(0,从缓存中删除设备信息成功.);
	 *		Result(1,从缓存中删除设备信息失败[需要删除的设备信息输入值为空]!);
	 *		Result(2,从缓存中删除设备信息失败[需要删除的设备信息输入值devOnlyId为空]!);
	 */
	list<bean.Result> batchDeleteDevice(1:string maxCode,2:string minCode,3:list<bean.DeviceCache> deviceCacheList)
	
	/**
	 * 批量删除设备信息，根据devOnlyId
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：115
	 * @param devOnlyIdList			需要删除的设备信息devOnlyId集合
	 * @return	
	 *		Result(0,从缓存中删除设备信息成功.);
	 *		Result(1,从缓存中删除设备信息失败[需要删除的设备信息输入值为空]!);
	 *		Result(2,从缓存中删除设备信息失败[需要删除的设备信息输入值devOnlyId为空]!);
	 */
	list<bean.Result> batchDeleteDeviceByDevOnlyId(1:string maxCode,2:string minCode,3:list<string> devOnlyIdList)
	
	
	
	/**
	 * 保存设备在线情况信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：201
	 * @param deviceOnlineCache	需要保存的设备在线情况信息
	 * @return
	 *		Result(0,保存设备在线情况信息到缓存成功.);
	 *		Result(1,保存设备在线情况信息到缓存失败[设备在线情况为空]!);
	 *		Result(2,保存设备在线情况信息到缓存失败[设备在线情况devOnlyId为空]!);
	 */
	bean.Result saveDeviceOnline(1:string maxCode,2:string minCode,3:bean.DeviceOnlineCache deviceOnlineCache)
	
	/**
	 * 更新设备在线情况信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：202
	 * @param deviceOnlineCache	需要更新的设备在线情况信息
	 * @return
	 *		Result(0,更新设备在线情况信息到缓存成功.);
	 *		Result(1,更新设备在线情况信息到缓存失败[设备在线情况信息为空]!);
	 *		Result(2,更新设备在线情况信息到缓存失败[设备在线情况devOnlyId为空]!);
	 */
	bean.Result updateDeviceOnline(1:string maxCode,2:string minCode,3:bean.DeviceOnlineCache deviceOnlineCache)
	
	/**
	 * 更新设备在线情况,只更新指定属性信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：203
	 * @param devOnlyId			需要更新设备devOnlyId
	 * @param fieldValueMap		需要更新设备在线情况的属性Key及Value值
	 * @return
	 *		Result(0,更新设备在线情况到缓存成功.);
	 *		Result(1,更新设备在线情况到缓存失败[设备在线情况属性Key及Value为空]!);
	 *		Result(2,更新设备在线情况到缓存失败[设备在线情况devOnlyId为空]!);
	 */
	bean.Result updateDeviceOnlineByField(1:string maxCode,2:string minCode,3:string devOnlyId,4:map<string,string> fieldValueMap )
	
	/**
	 * 查询设备在线情况
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：204
	 * @param deviceOnlineCache	需要查询的设备在线情况(至少devOnlyId不能为空)
	 * @return
	 *		DeviceOnlineCache	需要查询的设备在线情况
	 */
	bean.DeviceOnlineCache queryDeviceOnline(1:string maxCode,2:string minCode,3:bean.DeviceOnlineCache deviceOnlineCache)
	
	/**
	 * 查询设备在线情况，根据devOnlyId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：205
	 * @param devOnlyId			需要查询的设备devOnlyId
	 * @return
	 *		DeviceOnlineCache	需要查询的设备在线情况
	 */
	bean.DeviceOnlineCache queryDeviceOnlineByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 查询设备在线情况，根据IP 
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：206
	 * @param ip				需要查询的设备IP
	 * @return
	 *		DeviceOnlineCache	需要查询的设备在线情况信息
	 */
	bean.DeviceOnlineCache queryDeviceOnlineByIp(1:string maxCode,2:string minCode,3:string ip)
	
	/**
	 * 删除设备在线情况
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：207
	 * @param deviceOnlineCache		需要删除的在线设备情况(至少devOnlyId不能为空)
	 * @return
	 *		Result(0,从缓存中删除设备在线情况信息成功.);
	 *		Result(1,从缓存中删除设备在线情况信息失败[需要删除的设备信息输入值为空]!);
	 *		Result(2,从缓存中删除设备在线情况信息失败[需要删除的设备信息输入值devOnlyId为空]!);
	 */
	bean.Result deleteDeviceOnline(1:string maxCode,2:string minCode,3:bean.DeviceOnlineCache deviceOnlineCache)
	
	/**
	 * 删除设备在线情况，根据devOnlyId
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：208
	 * @param devOnlyId		需要删除的在线设备devOnlyId
	 * @return
	 *		Result(0,从缓存中删除设备在线情况信息成功.);
	 *		Result(1,从缓存中删除设备在线情况信息失败[需要删除的设备信息输入值devOnlyId为空]!);
	 */
	bean.Result deleteDeviceOnlineByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 删除设备在线情况，根据IP
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：209
	 * @param ip			需要删除的在线设备IP
	 * @return
	 *		Result(0,从缓存中删除设备在线情况信息成功.);
	 *		Result(1,从缓存中删除设备在线情况信息失败[需要删除的设备信息输入值ip为空]!);
	 */
	bean.Result deleteDeviceOnlineByIp(1:string maxCode,2:string minCode,3:string ip)
	
	/**
	 * 查询所有在线设备
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：210
	 * @return
	 *		list<bean.DeviceOnlineCache>		在线设备集合
	 */
	list<bean.DeviceOnlineCache> queryAllDeviceOnlineCache(1:string maxCode,2:string minCode)
	
	/**
     * 根据userOnlyId查询所有在线设备
     * @param maxCode		缓存服务号：00FF0700
     * @param minCode		缓存服务功能号：211
     * @return
     * 	list<String>		在线设备集合
     * 
     */
    list<string> queryOnlineDevOnlyIdsByUserOnlyId (1:string maxCode,2:string minCode,3:string userOnlyId);
	
	/**
     * 批量查询在线设备
     * @param maxCode		缓存服务号：00FF0700
     * @param minCode		缓存服务功能号：212
     * @return
     * 		list<bean.DeviceOnlineCache>		在线设备集合
     * 
     */
    list<bean.DeviceOnlineCache> batchQueryDeviceOnlinesByDevOnlyIdList (1:string maxCode,2:string minCode,3:list<string> devOnlyIdList);
	
	/**
	 * 保存设备会话密钥信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：301
	 * @param deviceKeyCache	需要保存的设备会话密钥信息
	 * @return
	 *		Result(0,保存设备会话密钥信息到缓存成功.);
	 *		Result(1,保存设备会话密钥信息到缓存失败[设备会话密钥信息为空]!);
	 *		Result(2,保存设备会话密钥信息到缓存失败[设备会话密钥信息sessionId为空]!);
	 */
	bean.Result saveDeviceKey(1:string maxCode,2:string minCode,3:bean.DeviceKeyCache deviceKeyCache)
	
	/**
	 * 更新设备会话密钥信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：302
	 * @param deviceKeyCache	需要更新的设备会话密钥信息
	 * @return
	 *		Result(0,更新设备会话密钥信息到缓存成功.);
	 *		Result(1,更新设备会话密钥信息到缓存失败[设备会话密钥信息为空]!);
	 *		Result(2,更新设备会话密钥信息到缓存失败[设备会话密钥信息sessionId为空]!);
	 */
	bean.Result updateDeviceKey(1:string maxCode,2:string minCode,3:bean.DeviceKeyCache deviceKeyCache)
	
	/**
	 * 更新设备会话密钥信息，更新指定属性
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：303
	 * @param sessionId			需要更新的设备会话sessionId
	 * @param fieldValueMap		需要更新的设备会话指定属性Key和Value值
	 * @return
	 *		Result(0,更新设备会话密钥信息到缓存成功.);
	 *		Result(1,更新设备会话密钥信息到缓存失败[设备会话密钥信息为空]!);
	 *		Result(2,更新设备会话密钥信息到缓存失败[设备会话密钥信息sessionId为空]!);
	 */
	bean.Result updateDeviceKeyByField(1:string maxCode,2:string minCode,3:string sessionId,4:map<string,string> fieldValueMap )
	
	/**
	 * 查询设备会话密钥信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：304
	 * @param deviceKeyCache	需要查询的设备会话密钥信息(至少sessionId不能为空)
	 * @return
	 *		DeviceKeyCache		需要查询的设备会话密钥信息
	 */
	bean.DeviceKeyCache queryDeviceKey(1:string maxCode,2:string minCode,3:bean.DeviceKeyCache deviceKeyCache)
	
	/**
	 * 查询设备会话密钥信息,根据会话sessionId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：305
	 * @param sessionId			需要查询的设备会话sessionId
	 * @return
	 *		DeviceKeyCache		需要查询的设备会话密钥信息
	 */
	bean.DeviceKeyCache queryDeviceKeyBySessionId(1:string maxCode,2:string minCode,3:string sessionId)
	
	/**
	 * 查询设备会话密钥信息,根据设备devOnlyId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：306
	 * @param devOnlyId			需要查询的设备devOnlyId
	 * @return
	 *		DeviceKeyCache		需要查询的设备会话密钥信息
	 */
	bean.DeviceKeyCache queryDeviceKeyByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 删除设备会话密钥信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：307
	 * @param deviceKeyCache	需要删除的设备会话密钥信息(至少sessionId不能为空)
	 * @return
	 *		Result(0,从缓存中删除设备会话密钥信息成功.);
	 *		Result(1,从缓存中删除设备会话密钥信息失败[需要删除的设备信息输入值sessionId为空]!);
	 */
	bean.Result deleteDeviceKey(1:string maxCode,2:string minCode,3:bean.DeviceKeyCache deviceKeyCache)
	
	/**
	 * 删除设备会话密钥信息,根据会话sessionId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：308
	 * @param sessionId			需要删除的设备会话sessionId
	 * @return
	 *		Result(0,从缓存中删除设备会话密钥信息成功.);
	 *		Result(1,从缓存中删除设备会话密钥信息失败[需要删除的设备信息输入值sessionId为空]!);
	 */
	bean.Result deleteDeviceKeyBySessionId(1:string maxCode,2:string minCode,3:string sessionId)
	
	/**
	 * 删除设备会话密钥信息,根据设备devOnlyId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：309
	 * @param devOnlyId			需要删除的设备devOnlyId
	 * @return
	 *		Result(0,从缓存中删除设备会话密钥信息成功.);
	 *		Result(1,从缓存中删除设备会话密钥信息失败[需要删除的设备信息输入值devOnlyId为空]!);
	 */
	bean.Result deleteDeviceKeyByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 验证sessionId 在DeviceKeyCache中是否存在
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：310
	 * @param sessionId			需要验证的sessionId
	 * @return
	 *		Result(0,存在.);
	 *		Result(1,不存在.);
	 *		Result(2,验证信息参数不正确[sessionId为空]!);
	 */
	bean.Result isExistSessionIdInDeviceKeyCache(1:string maxCode,2:string minCode,3:string sessionId)
	
	/**
	 * 批量查询设备会话秘钥信息集合
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：311
	 * @param List<String>		设备会话ID集合	
	 * @return
	 *		list<bean.DeviceKeyCache>	设备会话秘钥集合
	 */
	list<bean.DeviceKeyCache> batchQueryDeviceKeysBySessionIdList(1:string maxCode,2:string minCode,3:list<string> sessionIdList);
	
	/**
	 * 保存设备已安装产品信息
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：401
	 * @param devOnlyId				需要更新的设备devOnlyId
	 * @param deviceProducutList	需要保存的设备已安装产品信息
	 * @return
	 *		Result(0,保存设备已安装产品信息到缓存成功.);
	 *		Result(1,保存设备已安装产品信息到缓存失败[设备已安装产品信息为空]!);
	 *		Result(2,保存设备已安装产品信息到缓存失败[设备已安装产品信息devOnlyId为空]!);
	 */
	bean.Result saveDeviceInsProOld(1:string maxCode,2:string minCode,3:string devOnlyId,4:list<bean.DeviceProduct> deviceProductList)
	
	/**
	 * 更新设备已安装所有产品信息
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：402
	 * @param devOnlyId				需要更新的设备devOnlyId
	 * @param list<DeviceProduct>	需要更新的设备所有已安装产品信息(至少devOnlyId不能为空)
	 * @return
	 *		Result(0,更新设备已安装产品信息到缓存成功.);
	 *		Result(1,更新设备已安装产品信息到缓存失败[设备已安装产品信息为空]!);
	 *		Result(2,更新设备已安装产品信息到缓存失败[设备已安装产品信息devOnlyId为空]!);
	 */
	bean.Result updateDeviceInsProOld(1:string maxCode,2:string minCode,3:string devOnlyId,4:list<bean.DeviceProduct> deviceProductList)
	
	/**
	 * 查询设备已安装产品信息，根据设备devOnlyId
	 *
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：406
	 * @param devOnlyId		需要查询的设备devOnlyId
	 * @return
	 *		list<bean.DeviceProduct>	需要查询的设备所有已安装产品信息
	 */
	list<bean.DeviceProduct>	queryDeviceInsProOldByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 删除设备已安装产品信息，根据设备devOnlyId
	 *
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：408
	 * @param devOnlyId		需要删除的设备devOnlyId
	 * @return
	 *		Result(0,从缓存中删除设备已安装产品信息成功.);
	 *		Result(1,从缓存中删除设备已安装产品信息失败[设备已安装产品信息为空]!);
	 *		Result(2,从缓存中删除设备已安装产品信息失败[设备已安装产品信息devOnlyId为空]!);
	 */
	bean.Result deleteDeviceInsProOldByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	
	/**
	 * 保存设备应安装产品信息
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：501
	 * @param devOnlyId				需要保存的设备devOnlyId
	 * @param deviceProductList		需要保存的设备应安装产品信息
	 * @return
	 *		Result(0,保存设备应安装产品信息到缓存成功.);
	 *		Result(1,保存设备应安装产品信息到缓存失败[设备应安装产品信息为空]!);
	 *		Result(2,保存设备应安装产品信息到缓存失败[设备应安装产品信息devOnlyId为空]!);
	 */
	bean.Result saveDeviceInsProNew(1:string maxCode,2:string minCode,3:string devOnlyId,4:list<bean.DeviceProduct> deviceProductList)
	
	/**
	 * 更新设备应安装产品信息
	 * @param maxCode				缓存服务号：00FF0700
	 * @param minCode				缓存服务功能号：502
	 * @param devOnlyId				需要更新的设备devOnlyId
	 * @param deviceProductList		需要更新的设备应安装产品信息集合
	 * @return
	 *		Result(0,更新设备应安装产品信息到缓存成功.);
	 *		Result(1,更新设备应安装产品信息到缓存失败[设备应安装产品信息为空]!);
	 *		Result(2,更新设备应安装产品信息到缓存失败[设备应安装产品信息devOnlyId为空]!);
	 */
	bean.Result updateDeviceInsProNew(1:string maxCode,2:string minCode,3:string devOnlyId,4:list<bean.DeviceProduct> deviceProductList)
	
	/**
	 * 查询设备应安装产品信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：505
	 * @param devOnlyId		需要更新的设备devOnlyId
	 * @return
	 *		list<DeviceProduct>	设备应安装的产品信息集合
	 */
	list<bean.DeviceProduct> queryDeviceInsProNewByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 删除设备应安装所有产品信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：506
	 * @param devOnlyId		需要删除的设备devOnlyId
	 * @return
	 *		Result(0,从缓存中删除设备应安装所有产品信息成功.);
	 *		Result(1,从缓存中删除设备应安装所有产品信息失败[设备devOnlyId为空]!);
	 */
	bean.Result deleteDeviceInsProNewByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	
	/**
	 * 保存设备执行策略信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：601
	 * @param devicePolicyCache	需要保存的设备执行策略信息
	 * @return
	 *		Result(0,保存设备执行策略信息到缓存成功.);
	 *		Result(1,保存设备执行策略信息到缓存失败[设备执行策略信息为空]!);
	 *		Result(2,保存设备执行策略信息到缓存失败[设备执行策略信息devOnlyId为空]!);
	 */
	bean.Result saveDevicePolicy(1:string maxCode,2:string minCode,3:bean.DevicePolicyCache devicePolicyCache)
	
	/**
	 * 更新设备执行策略信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：602
	 * @param devicePolicyCache	需要更新的设备执行策略信息
	 * @return
	 *		Result(0,更新设备执行策略信息到缓存成功.);
	 *		Result(1,更新设备执行策略信息到缓存失败[设备执行策略信息为空]!);
	 *		Result(2,更新设备执行策略信息到缓存失败[设备执行策略信息devOnlyId为空]!);
	 */
	bean.Result updateDevicePolicy(1:string maxCode,2:string minCode,3:bean.DevicePolicyCache devicePolicyCache)
	
	/**
	 * 查询设备执行策略信息，根据设备devOnlyId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：603
	 * @param devOnlyId			需要查询的设备devOnlyId
	 * @return
	 *		DevicePolicyCache	需要查询的设备执行策略信息
	 */
	bean.DevicePolicyCache queryDevicePolicyByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 查询设备执行策略信息，根据设备IP 
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：604
	 * @param ip				需要查询的设备IP 
	 * @return
	 *		DevicePolicyCache	需要查询的设备执行策略信息
	 */
	bean.DevicePolicyCache queryDevicePolicyByIp(1:string maxCode,2:string minCode,3:string ip)
	
	/**
	 * 删除设备执行策略信息，根据设备devOnlyId
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：605
	 * @param devOnlyId			需要删除的设备devOnlyId
	 * @return
	 *		Result(0,从缓存中删除设备执行策略信息成功.);
	 *		Result(1,从缓存中删除设备执行策略信息失败[设备执行策略信息为空]!);
	 *		Result(2,从缓存中删除设备执行策略信息失败[设备执行策略信息devOnlyId为空]!);
	 */
	bean.Result deleteDevicePolicyByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 删除设备执行策略信息，根据设备IP
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：606
	 * @param ip				需要删除的设备IP
	 * @return
	 *		Result(0,从缓存中删除设备执行策略信息成功.);
	 *		Result(1,从缓存中删除设备执行策略信息失败[设备IP为空]!);
	 */
	bean.Result deleteDevicePolicyByIp(1:string maxCode,2:string minCode,3:string ip)
	
	/**
	 * 批量保存用户策略信息
	 * @param maxCode		       缓存服务号：00FF0700
	 * @param minCode		       缓存服务功能号：607
	 * @param devicePolicyList     设备策略集合信息
	 * @return
	 *		Result(0,保存设备策略信息到缓存成功.);
	 *		Result(1,保存设备策略信息到缓存失败[功能号minCode不正确]!);
	 *		Result(2,保存设备策略信息到缓存失败[设备策略集合信息为空]!);
	 */
	bean.Result batchSaveDevicePolicy(1:string maxCode,2:string minCode,3:list<map<string,string>> devicePolicyList)
	
	/**
     * 保存用户信息
     * @param maxCode		缓存服务号：00FF0700
     * @param minCode		缓存服务功能号：701
     * @param userCache		需要保存的用户信息
     * @return
     * 	Result(0,保存用户信息到缓存成功.);
     * 	Result(1,保存用户信息到缓存失败[用户信息为空]!);
     * 	Result(2,保存用户信息到缓存失败[用户信息userOnlyId为空]!);
     * 
     * @param maxCode
     * @param minCode
     * @param userCache
     */
    bean.Result saveUser(1:string maxCode, 2:string minCode, 3:bean.UserCache userCache)
	
	/**
	 * 更新用户信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：702
	 * @param userCache		需要更新的用户信息
	 * @return
	 *		Result(0,更新用户信息到缓存成功.);
	 *		Result(1,更新用户信息到缓存失败[用户信息为空]!);
	 *		Result(2,更新用户信息到缓存失败[用户信息userOnlyId为空]!);
	 */
	bean.Result updateUser(1:string maxCode,2:string minCode,3:bean.UserCache userCache)
	
	/**
	 * 更新用户信息,更新指定属性
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：703
	 * @param userCache		需要更新的用户信息
	 * @return
	 *		Result(0,更新用户信息到缓存成功.);
	 *		Result(1,更新用户信息到缓存失败[用户信息userOnlyId为空]!);
	 *		Result(2,更新用户信息到缓存失败[需要更新的指定属性Key和Value为空]!);
	 */
	bean.Result updateUserByField(1:string maxCode,2:string minCode,3:string userOnlyId,4:map<string,string> fieldValueMap )
	
	/**
	 * 查询用户信息，根据userOnlyId
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：704
	 * @param userOnlyId	需要查询的用户userOnlyId
	 * @return
	 *		UserCache		需要查询的用户信息
	 */
	bean.UserCache queryUserByUserOnlyId(1:string maxCode,2:string minCode,3:string userOnlyId)
	
	/**
	 * 删除用户信息，根据userOnlyId
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：705
	 * @param userOnlyId	需要删除的用户userOnlyId
	 * @return
	 *		Result(0,从缓存中删除用户信息成功.);
	 *		Result(1,从缓存中删除用户信息失败[用户信息userOnlyId为空]!);
	 */
	bean.Result deleteUserByUserOnlyId(1:string maxCode,2:string minCode,3:string userOnlyId)
	
	/**
	 * 批量保存用户信息
	 * @param maxCode		   缓存服务号：00FF0700
	 * @param minCode		   缓存服务功能号：706
	 * @param list<bean.UserCache>  需要保存的用户信息
	 * @return
	 *		Result(0,保存用户信息到缓存成功.);
	 *		Result(1,保存用户信息到缓存失败[minCode为空]!);
	 *		Result(2,批量保存用户信息到缓存失败[用户信息list<bean.UserCache>为空]!);
	 */
	bean.Result batchSaveUser(1:string maxCode,2:string minCode,3:list<bean.UserCache> userCacheList)
	
	/**
	 * 保存用户在线情况信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：801
	 * @param userOnlineCache	需要保存的用户在线情况信息
	 * @return
	 *		Result(0,保存用户在线情况信息到缓存成功.);
	 *		Result(1,保存用户在线情况信息到缓存失败[用户在线情况信息为空]!);
	 *		Result(2,保存用户在线情况信息到缓存失败[用户在线情况信息中userOnlyId为空]!);
	 *		Result(11,保存用户在线情况信息到缓存失败[用户在线情况信息中devOnlyId为空]!);
	 *		Result(12,保存用户在线情况信息到缓存失败[用户在线情况信息中userDevId为空]!);
	 */
	bean.Result saveUserOnline(1:string maxCode,2:string minCode,3:bean.UserOnlineCache userOnlineCache)
	
	/**
	 * 更新用户在线情况信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：802
	 * @param userOnlineCache	需要更新的用户在线情况信息
	 * @return
	 *		Result(0,更新用户在线情况信息到缓存成功.);
	 *		Result(1,更新用户在线情况信息到缓存失败[用户在线情况信息为空]!);
	 *		Result(2,更新用户在线情况信息到缓存失败[用户在线情况信息中userOnlyId为空]!);
	 *		Result(11,更新用户在线情况信息到缓存失败[用户在线情况信息中devOnlyId为空]!);
	 *		Result(12,更新用户在线情况信息到缓存失败[用户在线情况信息中userDevId为空]!);
	 */
	bean.Result updateUserOnline(1:string maxCode,2:string minCode,3:bean.UserOnlineCache userOnlineCache)
	
	/**
	 * 更新用户在线情况信息，更新指定属性
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：803
	 * @param userDevId			需要更新的用户在线Key：userDevId
	 * @param fieldValueMap		需要更新的用户在线Key对应的属性信息集合
	 * @return
	 *		Result(0,更新用户在线情况信息到缓存成功.);
	 *		Result(1,更新用户在线情况信息到缓存失败[被更新Key：userDevId为空]!);
	 *		Result(2,更新用户在线情况信息到缓存失败[用户在线情况信息中，被指定更新的属性Key和Value为空]!);
	 */
	bean.Result updateUserOnlineByField(1:string maxCode,2:string minCode,3:string userDevId,4:map<string,string> fieldValueMap )
	
	/**
	 * 查询用户登陆设备的在线情况信息，根据userDevId
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：804
	 * @param userDevId		需要查询的用户登陆设备Key：userDevId
	 * @return
	 *		UserOnlieCache	用户在线情况信息
	 */
	bean.UserOnlineCache queryUserOnlineByUserDevId(1:string maxCode,2:string minCode,3:string userDevId)
	
	/**
	 * 删除用户登陆设备在线情况信息，根据用户登陆设备在线Key：userDevId
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：805
	 * @param userDevId		需要删除的用户登陆设备在线Key：userDevId
	 * @return
	 *		Result(0,从缓存中删除用户在线情况信息成功.);
	 *		Result(1,从缓存中删除用户在线情况信息失败[用户登陆设备在线Key：userDevId为空]!);
	 */
	bean.Result deleteUserOnlineByUserDevId(1:string maxCode,2:string minCode,3:string userDevId)
	
	/**
	 * 查询所有用户在线设备信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：806
	 * @return	map<string, set<string>>   所有用户在线设备信息
	 */
	map<string, set<string>> queryAllUserOnlineDevice(1:string maxCode,2:string minCode);

	/**
	 * 保存用户执行策略信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：901
	 * @param userPolicyCache	需要保存的用户执行策略信息
	 * @return
	 *		Result(0,保存用户执行策略信息到缓存成功.);
	 *		Result(1,保存用户执行策略信息到缓存失败[用户执行策略信息为空]!);
	 *		Result(2,保存用户执行策略信息到缓存失败[用户执行策略信息userOnlyId为空]!);
	 */
	bean.Result saveUserPolicy(1:string maxCode,2:string minCode,3:bean.UserPolicyCache userPolicyCache)
	
	/**
	 * 更新用户执行策略信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：902
	 * @param userPolicyCache	需要更新的用户执行策略信息
	 * @return
	 *		Result(0,更新用户执行策略信息到缓存成功.);
	 *		Result(1,更新用户执行策略信息到缓存失败[用户执行策略信息为空]!);
	 *		Result(2,更新用户执行策略信息到缓存失败[用户执行策略信息userOnlyId为空]!);
	 */
	bean.Result updateUserPolicy(1:string maxCode,2:string minCode,3:bean.UserPolicyCache userPolicyCache)
	
	/**
	 * 查询用户执行策略信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：903
	 * @param userOnlyId	需要查询的用户userOnlyId
	 * @return
	 *		UserPolicyCache	需要查询的用户执行策略信息
	 */
	bean.UserPolicyCache queryUserPolicyByUserOnlyId(1:string maxCode,2:string minCode,3:string userOnlyId)
	
	/**
	 * 删除用户执行策略信息，根据userOnlyId
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：904
	 * @param userOnlyId	需要删除的用户userOnlyId
	 * @return
	 *		Result(0,从缓存中删除用户执行策略信息成功.);
	 *		Result(1,从缓存中删除用户执行策略信息失败[用户执行策略信息userOnlyId为空]!);
	 */
	bean.Result deleteUserPolicyByUserOnlyId(1:string maxCode,2:string minCode,3:string userOnlyId)

	/**
	 * 批量保存用户执行策略信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：905
	 * @param userPolicyist 用户策略集合
	 * @return
	 *		Result(0,批量保存用户执行策略信息成功！);
	 *		Result(1,批量保存用户执行策略信息失败[功能号minCode不正确]！);
	 *      Result(2,批量保存用户策略集合失败[用户策略集合为空]！);
	 */
	bean.Result batchSaveUserPolicy(1:string maxCode,2:string minCode,3:list<map<string,string>> userPolicyist)
	
	/**
	 * 保存产品安装包信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1001
	 * @param cInstallPackCache	需要保存的产品安装包信息
	 * @return
	 *		Result(0,保存产品安装包信息到缓存成功.);
	 *		Result(1,保存产品安装包信息到缓存失败[产品信息为空]!);
	 *		Result(2,保存产品安装包信息到缓存失败[产品信息cInstallPackId为空]!);
	 */
	bean.Result saveProductCInstallPack(1:string maxCode,2:string minCode,3:bean.CInstallPackCache cInstallPackCache)
	
	/**
	 * 更新产品安装包信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1002
	 * @param cInstallPackCache	需要更新的产品安装包信息
	 * @return
	 *		Result(0,更新产品安装包信息到缓存成功.);
	 *		Result(1,更新产品安装包信息到缓存失败[产品信息为空]!);
	 *		Result(2,更新产品安装包信息到缓存失败[产品信息cInstallPackId为空]!);
	 */
	bean.Result updateProductCInstallPack(1:string maxCode,2:string minCode,3:bean.CInstallPackCache cInstallPackCache)
	
	/**
	 * 更新产品安装包信息,指定属性更新
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1003
	 * @param cInstallPackId	需要更新的产品安装包ID
	 * @param fieldValueMap		需要更新的指定属性Key和Value
	 * @return
	 *		Result(0,更新产品安装包信息到缓存成功.);
	 *		Result(1,更新产品安装包信息到缓存失败[产品信息为空]!);
	 *		Result(2,更新产品安装包信息到缓存失败[产品信息cInstallPackId为空]!);
	 */
	bean.Result updateProductCInstallPackByField(1:string maxCode,2:string minCode,3:string cInstallPackId,4:map<string,string> fieldValueMap )
	
	/**
	 * 查询产品安装包信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1004
	 * @param cInstallPackId	需要查询的产品安装包ID
	 * @return
	 *		CInstallPackCache	需要查询的产品安装包信息
	 */
	bean.CInstallPackCache queryProductCInstallPackById(1:string maxCode,2:string minCode,3:string cInstallPackId)
	
	/**
	 * 删除产品安装包信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1005
	 * @param cInstallPackId	需要更新的产品安装包ID
	 * @return
	 *		Result(0,从缓存中删除产品安装包信息成功.);
	 *		Result(1,从缓存中删除产品安装包信息失败[产品信息cInstallPackId为空]!);
	 */
	bean.Result deleteProductCInstallPackById(1:string maxCode,2:string minCode,3:string cInstallPackId)
	
	
	/**
	 * 保存策略信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1101
	 * @param policyCache	需要保存的策略信息
	 * @return
	 *		Result(0,保存策略信息到缓存成功.);
	 *		Result(1,保存策略信息到缓存失败[策略信息为空]!);
	 *		Result(2,保存策略信息到缓存失败[策略信息policyId为空]!);
	 */
	bean.Result savePolicy(1:string maxCode,2:string minCode,3:bean.PolicyCache policyCache)
	
	/**
	 * 更新策略信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1102
	 * @param policyCache	需要保存的策略信息
	 * @return
	 *		Result(0,更新策略信息到缓存成功.);
	 *		Result(1,更新策略信息到缓存失败[策略信息为空]!);
	 *		Result(2,更新策略信息到缓存失败[策略信息policyId为空]!);
	 */
	bean.Result updatePolicy(1:string maxCode,2:string minCode,3:bean.PolicyCache policyCache)
	
	/**
	 * 更新策略信息，更新指定的属性信息
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1103
	 * @param policyCache	需要更新的策略信息
	 * @return
	 *		Result(0,更新策略信息到缓存成功.);
	 *		Result(1,更新策略信息到缓存失败[策略信息policyId为空]!);
	 *		Result(2,更新策略信息到缓存失败[指定更新的策略属性信息Key和Value为空]!);
	 */
	bean.Result updatePolicyByField(1:string maxCode,2:string minCode,3:string policyId,4:map<string,string> fieldValueMap )
	
	/**
	 * 查询策略信息，根据策略ID 
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1104
	 * @param policyId		需要查询的策略ID
	 * @return
	 *		PolicyCache	需要查询的策略信息
	 */
	bean.PolicyCache queryPolicyByPolicyId(1:string maxCode,2:string minCode,3:string policyId)
	
	/**
	 * 删除策略信息，根据策略ID 
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1105
	 * @param policyId		需要删除的策略ID
	 * @return
	 *		Result	需要查询的策略信息
	 */
	bean.Result deletePolicyByPolicyId(1:string maxCode,2:string minCode,3:string policyId)
	
	/**
	 * 保存IPMAC与devOnlyId的对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1201
	 * @param ip			IP 
	 * @param mac			MAC
	 * @param devOnlyId		设备唯一ID
	 * @return
	 *		Result(0,保存IPMAC与devOnlyId的对应关系成功.);
	 *		Result(1,保存IPMAC与devOnlyId的对应关系失败[IP为空]!);
	 *		Result(2,保存IPMAC与devOnlyId的对应关系失败[MAC为空]!);
	 *		Result(3,保存IPMAC与devOnlyId的对应关系失败[devOnlyId为空]!);
	 */
	bean.Result saveIPMAC2DevOnlyId(1:string maxCode,2:string minCode,3:string ip,4:string mac,5:string devOnlyId)
	
	/**
	 * 更新IPMAC与devOnlyId的对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1202
	 * @param ip			IP 
	 * @param mac			MAC
	 * @param devOnlyId		设备唯一ID
	 * @return
	 *		Result(0,更新IPMAC与devOnlyId的对应关系成功.);
	 *		Result(1,更新IPMAC与devOnlyId的对应关系失败[IP为空]!);
	 *		Result(2,更新IPMAC与devOnlyId的对应关系失败[MAC为空]!);
	 *		Result(3,更新IPMAC与devOnlyId的对应关系失败[devOnlyId为空]!);
	 */
	bean.Result updateIPMAC2DevOnlyId(1:string maxCode,2:string minCode,3:string ip,4:string mac,5:string devOnlyId)
	
	/**
	 * 查询IPMAC与devOnlyId的对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1203
	 * @param ip			IP 
	 * @param mac			MAC
	 * @return
	 *		devOnlyId		设备唯一ID
	 */
	string queryIPMAC2DevOnlyId(1:string maxCode,2:string minCode,3:string ip,4:string mac)
	
	/**
	 * 删除IPMAC与devOnlyId的对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1204
	 * @param ip			IP 
	 * @param mac			MAC
	 * @return
	 *		Result(0,删除IPMAC与devOnlyId的对应关系成功.);
	 *		Result(1,删除IPMAC与devOnlyId的对应关系失败[IP为空]!);
	 *		Result(2,删除IPMAC与devOnlyId的对应关系失败[MAC为空]!);
	 */
	bean.Result deleteIPMAC2DevOnlyId(1:string maxCode,2:string minCode,3:string ip,4:string mac)
	
	/**
	 * 保存IP与devOnlyId对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1301
	 * @param ip			IP
	 * @param devOnlyId		设备唯一ID
	 * @return
	 *		Result(0,保存IP与devOnlyId对应关系到缓存成功.);
	 *		Result(1,保存IP与devOnlyId对应关系到缓存失败[IP为空]!);
	 *		Result(2,保存IP与devOnlyId对应关系到缓存失败[devOnlyId为空]!);
	 */
	bean.Result saveIP2DevOnlyId(1:string maxCode,2:string minCode,3:string ip,4:string devOnlyId)
	
	/**
	 * 更新IP与devOnlyId对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1302
	 * @param ip			IP
	 * @param devOnlyId		设备唯一ID
	 * @return
	 *		Result(0,更新IP与devOnlyId对应关系到缓存成功.);
	 *		Result(1,更新IP与devOnlyId对应关系到缓存失败[IP为空]!);
	 *		Result(2,更新IP与devOnlyId对应关系到缓存失败[devOnlyId为空]!);
	 */
	bean.Result updateIP2DevOnlyId(1:string maxCode,2:string minCode,3:string ip,4:string devOnlyId)
	
	/**
	 * 查询IP与devOnlyId对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1303
	 * @param ip			IP
	 * @return
	 *		devOnlyId		IP对应的设备唯一ID
	 */
	string queryIP2DevOnlyId(1:string maxCode,2:string minCode,3:string ip)
	
	/**
	 * 删除IP与devOnlyId对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1304
	 * @param ip			IP
	 * @return
	 *		Result(0,从缓存中删除IP与devOnlyId对应关系成功.);
	 *		Result(1,从缓存中删除IP与devOnlyId对应关系失败[IP为空]!);
	 */
	bean.Result deleteIP2DevOnlyId(1:string maxCode,2:string minCode,3:string ip)
  
	/**
	 * 保存产品升级包信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1401
	 * @param cUpgradePackCache	需要保存的产品升级包信息
	 * @return
	 *		Result(0,保存产品升级包信息到缓存成功.);
	 *		Result(1,保存产品升级包信息到缓存失败[产品信息为空]!);
	 *		Result(2,保存产品升级包信息到缓存失败[产品信息cUpgradePackId为空]!);
	 */
	bean.Result saveProductCUpgradePack(1:string maxCode,2:string minCode,3:bean.CUpgradePackCache cUpgradePackCache)
	
	/**
	 * 更新产品升级包信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1402
	 * @param cUpgradePackCache	需要更新的产品升级包信息
	 * @return
	 *		Result(0,更新产品升级包信息到缓存成功.);
	 *		Result(1,更新产品升级包信息到缓存失败[产品信息为空]!);
	 *		Result(2,更新产品升级包信息到缓存失败[产品信息cUpgradePackId为空]!);
	 */
	bean.Result updateProductCUpgradePack(1:string maxCode,2:string minCode,3:bean.CUpgradePackCache cUpgradePackCache)
	
	/**
	 * 更新产品升级包信息,指定属性更新
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1403
	 * @param cUpgradePackId	需要更新的产品升级包ID
	 * @param fieldValueMap		需要更新的指定属性Key和Value
	 * @return
	 *		Result(0,更新产品升级包信息到缓存成功.);
	 *		Result(1,更新产品升级包信息到缓存失败[产品信息为空]!);
	 *		Result(2,更新产品升级包信息到缓存失败[产品信息cUpgradePackId为空]!);
	 */
	bean.Result updateProductCUpgradePackByField(1:string maxCode,2:string minCode,3:string cUpgradePackId,4:map<string,string> fieldValueMap )
	
	/**
	 * 查询产品升级包信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1404
	 * @param cUpgradePackId	需要查询的产品升级包ID
	 * @return
	 *		CUpgradePackCache	需要查询的产品升级包信息
	 */
	bean.CUpgradePackCache queryProductCUpgradePackById(1:string maxCode,2:string minCode,3:string cUpgradePackId)
	
	/**
	 * 删除产品升级包信息
	 * @param maxCode			缓存服务号：00FF0700
	 * @param minCode			缓存服务功能号：1405
	 * @param cUpgradePackId	需要更新的产品升级包ID
	 * @return
	 *		Result(0,从缓存中删除产品升级包信息成功.);
	 *		Result(1,从缓存中删除产品升级包信息失败[产品信息cUpgradePackId为空]!);
	 */
	bean.Result deleteProductCUpgradePackById(1:string maxCode,2:string minCode,3:string cUpgradePackId)
	
	
	/**
	 * 保存account与userOnlyId对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1501
	 * @param account			用户账号
	 * @param userOnlyId		用户唯一ID
	 * @return
	 *		Result(0,保存account与userOnlyId对应关系到缓存成功.);
	 *		Result(1,保存account与userOnlyId对应关系到缓存失败[account为空]!);
	 *		Result(2,保存account与userOnlyId对应关系到缓存失败[userOnlyId为空]!);
	 */
	bean.Result saveAccount2UserOnlyId(1:string maxCode,2:string minCode,3:string account,4:string userOnlyId)
	
	/**
	 * 更新account与userOnlyId对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1502
	 * @param account			用户账号
	 * @param userOnlyId	设备唯一ID
	 * @return
	 *		Result(0,更新account与userOnlyId对应关系到缓存成功.);
	 *		Result(1,更新account与userOnlyId对应关系到缓存失败[account为空]!);
	 *		Result(2,更新account与userOnlyId对应关系到缓存失败[userOnlyId为空]!);
	 */
	bean.Result updateAccount2UserOnlyId (1:string maxCode,2:string minCode, 3:string account,4:string userOnlyId)
	
	/**
	 * 查询account与userOnlyId对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1503
	 * @param account			用户账号
	 * @return
	 *		userOnlyId	用户唯一ID
	 */
	string queryAccount2UserOnlyId (1:string maxCode,2:string minCode,3:string account)
	
	/**
	 * 删除account与userOnlyId对应关系
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1504
	 * @param account		用户账号
	 * @return
	 *		Result(0,从缓存中删除account与userOnlyId对应关系成功.);
	 *		Result(1,从缓存中删除account与userOnlyId对应关系失败[account为空]!);
	 */
	bean.Result deleteAccount2UserOnlyId (1:string maxCode,2:string minCode,3:string account)
	
	/**
	 * 批量保存account与userOnlyId对应关系
	 * @param maxCode					缓存服务号：00FF0700
	 * @param minCode					缓存服务功能号：1505
	 * @param list<bean.UserCache>		用户集合
	 * @return
	 *		Result(0,保存account与userOnlyId对应关系到缓存成功.);
	 *		Result(1,保存account与userOnlyId对应关系到缓存失败[minCode为空]!);
	 *		Result(2,保存account与userOnlyId对应关系到缓存失败[list<bean.UserCache>为空]!);
	 */
	bean.Result batchSaveAccount2UserOnlyId(1:string maxCode,2:string minCode,3:list<bean.UserCache> userCacheList)
	
	/**
	 * 保存点对点缓存数据
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1601
	 * @param uuid			MD5(maxCode+minCode+devOnlyId )
	 * @param ptpResult		点对点操作返回结果
	 * @return
	 *		Result(0,保存点对点缓存数据成功.);
	 *		Result(1,保存点对点缓存数据失败[uuid为空]!);
	 */
	bean.Result savePtp(1:string maxCode,2:string minCode,3:string uuid,4:string ptpResult)
	
	/**
	 * 更新点对点缓存数据
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1602
	 * @param uuid			MD5(maxCode+minCode+devOnlyId )
	 * @param ptpResult		点对点操作返回结果
	 * @return
	 *		Result(0,更新点对点缓存数据成功.);
	 *		Result(1,更新点对点缓存数据[uuid为空]!);
	 */
	bean.Result updatePtp (1:string maxCode,2:string minCode, 3:string uuid,4:string ptpResult)
	
	/**
	 * 查询点对点缓存数据
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1603
	 * @param uuid			MD5(maxCode+minCode+devOnlyId )
	 * @return
	 *		ptpResult  点对点操作客户端反馈结果
	 */
	string queryPtp(1:string maxCode,2:string minCode, 3:string uuid)
	
	/**
	 * 删除点对点数据
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1604
	 * @param uuid			MD5(maxCode+minCode+devOnlyId )
	 * @return
	 *		Result(0,从缓存中删除点对点缓存数据成功.);
	 *		Result(1,从缓存中删除点对点缓存数据失败[uuid为空]!);
	 */
	bean.Result deletePtp (1:string maxCode,2:string minCode,3:string uuid)
	
	
	/**
	 * 保存敏感规则库信息缓存数据
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1701
	 * @param sensitiveCache	敏感规则库信息缓存数据
	 * @return
	 *		Result(0,保存敏感规则库信息缓存数据成功.);
	 *		Result(1,保存敏感规则库信息数据失败[name为空]!);
	 */
	bean.Result saveSensitive(1:string maxCode,2:string minCode,3:bean.SensitiveCache sensitiveCache)
	/**
	 * 更新敏感规则库信息缓存数据
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1702
	 * @param sensitiveCache	敏感规则库信息缓存数据
	 * @return
	 *		Result(0,更新敏感规则库信息缓存数据成功.);
	 *		Result(1,更新敏感规则库信息数据失败[name为空]!);
	 */
	bean.Result updateSensitive (1:string maxCode,2:string minCode, 3:bean.SensitiveCache sensitiveCache)
	/**
	 * 查询敏感规则库信息缓存数据
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1703
	 * @param name			敏感规则库文件名
	 * @return
	 *		SensitiveCache	敏感规则库信息缓存数据
	 */
	bean.SensitiveCache querySensitive (1:string maxCode,2:string minCode, 3:string name)
	/**
	 * 查询敏感规则库信息缓存数据，根据name查询crc
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1704
	 * @param name			敏感规则库文件名
	 * @param fieldKey		需要查询的字段键值(eg. crc,path...)
	 * @return
	 *		fieldValue 		需要查询的字段对应的值
	 */
	string querySensitiveByField (1:string maxCode,2:string minCode,3:string name,4:string fieldKey)
	/**
	 * 删除敏感规则库信息缓存数据
	 * @param maxCode		缓存服务号：00FF0700
	 * @param minCode		缓存服务功能号：1705
	 * @param name			敏感规则库文件名
	 * @return
	 *		Result(0,删除敏感规则库信息缓存数据成功.);
	 *		Result(1,删除敏感规则库信息数据失败[name为空]!);
	 */
	bean.Result deleteSensitive (1:string maxCode,2:string minCode,3:string name)
	
	/**
	 * 保存设备消息摘要信息
	 * @param maxCode		 缓存服务号：00FF0700
	 * @param minCode		 缓存服务功能号：1801
	 * @param devOnlyId 	 设备唯一ID
	 * @param fieldValueMap	 消息摘要(key为消息类型，value为消息ID集合)
	 * @return
	 *		Result(0,保存设备消息摘要信息成功.);
	 *		Result(1,保存设备消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,保存设备消息摘要信息失败[devOnlyId或fieldValueMap为空]!)
	 */
	bean.Result saveDeviceMsgDigest(1:string maxCode,2:string minCode,3:string devOnlyId,4:map<string,list<string>> fieldValueMap)
	
	/**
	 * 保存设备的某一类型消息摘要信息
	 * @param maxCode		 缓存服务号：00FF0700
	 * @param minCode		 缓存服务功能号：1802
	 * @param devOnlyId 	 设备唯一ID
	 * @param msgType	 	 消息类型
	 * @param msgIdList	 	 消息ID集合
	 * @return
	 *		Result(0,保存设备消息摘要信息成功.);
	 *		Result(1,保存设备消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,保存设备消息摘要信息失败[devOnlyId、msgType或msgIdList为空]!)
	 */
	bean.Result saveDeviceMsgDigestWithDevOnlyIdAndMsgType(1:string maxCode,2:string minCode,3:string devOnlyId,4:string msgType,5:list<string> msgIdList)
	
	/**
	 * 保存设备的某一类型消息的某一个摘要信息
	 * @param maxCode		 缓存服务号：00FF0700
	 * @param minCode		 缓存服务功能号：1803
	 * @param devOnlyId 	 设备唯一ID
	 * @param msgType	 	 消息类型
	 * @param msgId	 	 	 消息ID
	 * @return
	 *		Result(0,保存设备消息摘要信息成功.);
	 *		Result(1,保存设备消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,保存设备消息摘要信息失败[devOnlyId、msgType或msgId为空]!)
	 */
	bean.Result saveDeviceMsgDigestWithDevOnlyIdAndMsgTypeAndMsgId(1:string maxCode,2:string minCode,3:string devOnlyId,4:string msgType,5:string msgId)
	
	/**
	 * 删除某一设备的所有消息类别的消息摘要信息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1804
	 * @param devOnlyId	         设备唯一ID
	 * @return
	 *		Result(0,删除设备消息摘要信息成功.);
	 *		Result(1,删除设备消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,删除设备消息摘要信息失败[devOnlyId值为空]!);
	 */
	bean.Result deleteDeviceMsgDigestByDevOnlyId (1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 删除某一设备的某一消息类别的消息摘要信息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1805
	 * @param devOnlyId	         设备唯一ID
	 * @param msgType            消息类型
	 * @return
	 *		Result(0,删除设备消息摘要信息成功.);
	 *		Result(1,删除设备消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,删除设备消息摘要信息失败[devOnlyId或msgType值为空]!);
	 */
	bean.Result deleteDeviceMsgDigestByDevOnlyIdAndMsgType (1:string maxCode,2:string minCode,3:string devOnlyId,4:string msgType)
	
	/**
	 * 删除某一设备的某一消息类别中某一个消息摘要信息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1806
	 * @param devOnlyId	         设备唯一ID
	 * @param msgType            消息类型
	 * @param msgId	 	 	 	 消息ID
	 * @return
	 *		Result(0,删除设备消息摘要信息成功.);
	 *      Result(1,删除设备消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(1,删除设备消息摘要信息失败[devOnlyId或msgType或msgId值为空]!);
	 */
	bean.Result deleteDeviceMsgDigestByDevOnlyIdAndMsgTypeAndMsgId (1:string maxCode,2:string minCode,3:string devOnlyId,4:string msgType,5:string msgId)
	
	/**
	 * 更新某一设备的某一消息类别的消息摘要信息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1807
	 * @param devOnlyId     	 设备唯一ID
	 * @param msgType     	     设备类型
	 * @param msgId     	     消息Id
	 * @return
	 *		Result(0,更新设备消息摘要成功.);
	 *		Result(1,更新设备消息摘要失败[minCode功能号不正确]!);
	 *		Result(2,更新设备消息摘要失败[devOnlyId、msgType或msgId为空]!);
	 */
	bean.Result updateDeviceMsgDigest(1:string maxCode,2:string minCode,3:string devOnlyId,4:string msgType,5:string msgId)
	
	/**
	 * 查询某一设备的所有消息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1808
	 * @param devOnlyId	         设备唯一ID
	 * @return
	 *		map<string,list<string>>    某一设备的所有消息
	 */
	map<string,list<string>> queryDeviceMsgDigestByDevOnlyId(1:string maxCode,2:string minCode,3:string devOnlyId)
	
	/**
	 * 查询某一设备的某一消息类别的所有消息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1809
	 * @param devOnlyId	         设备唯一ID
	 * @param msgType			 消息类别
	 * @return
	 *		list<string>         同一设备的所有消息
	 */
	list<string> queryDeviceMsgDigestByDevIdAndMsgType(1:string maxCode,2:string minCode,3:string devOnlyId,4:string msgType)
	
	/**
	 * 保存用户消息摘要信息
	 * @param maxCode		 缓存服务号：00FF0700
	 * @param minCode		 缓存服务功能号：1901
	 * @param userOnlyId 	 用户唯一ID
	 * @param fieldValueMap	 消息摘要(key为消息类型，value为消息ID集合)
	 * @return
	 *		Result(0,保存用户消息摘要信息成功.);
	 *		Result(1,保存用户消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,保存用户消息摘要信息失败[userOnlyId或fieldValueMap为空]!)
	 */
	bean.Result saveUserMsgDigest(1:string maxCode,2:string minCode,3:string userOnlyId,4:map<string,list<string>> fieldValueMap)
	
	/**
	 * 保存用户的某一类型消息摘要信息
	 * @param maxCode		 缓存服务号：00FF0700
	 * @param minCode		 缓存服务功能号：1902
	 * @param userOnlyId 	 用户唯一ID
	 * @param msgType	 	 消息类型
	 * @param msgIdList	 	 消息ID集合
	 * @return
	 *		Result(0,保存用户消息摘要信息成功.);
	 *		Result(1,保存用户消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,保存用户消息摘要信息失败[userOnlyId、msgType或msgIdList为空]!)
	 */
	bean.Result saveUserMsgDigestWithUserOnlyIdAndMsgType(1:string maxCode,2:string minCode,3:string userOnlyId,4:string msgType,5:list<string> msgIdList)
	
	/**
	 * 保存用户的某一类型消息的某一个摘要信息
	 * @param maxCode		 缓存服务号：00FF0700
	 * @param minCode		 缓存服务功能号：1903
	 * @param userOnlyId 	 用户唯一ID
	 * @param msgType	 	 消息类型
	 * @param msgId	 	 	 消息ID
	 * @return
	 *		Result(0,保存用户消息摘要信息成功.);
	 *		Result(1,保存用户消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,保存用户消息摘要信息失败[userOnlyId、msgType或msgId为空]!)
	 */
	bean.Result saveUserMsgDigestWithUserOnlyIdAndMsgTypeAndMsgId(1:string maxCode,2:string minCode,3:string userOnlyId,4:string msgType,5:string msgId)
	
	/**
	 * 删除某一用户的所有消息类别的消息摘要信息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1904
	 * @param userOnlyId	         设备唯一ID
	 * @return
	 *		Result(0,删除用户消息摘要信息成功.);
	 *      Result(1,删除用户消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,删除用户消息摘要信息失败[userOnlyId值为空]!);      
	 */
	bean.Result deleteUserMsgDigestByUserOnlyId (1:string maxCode,2:string minCode,3:string userOnlyId)
	
	/**
	 * 删除某一用户的某一消息类别的消息摘要信息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1905
	 * @param userOnlyId	     用户唯一ID
	 * @param msgType            消息类型
	 * @return
	 *		Result(0,删除用户消息摘要信息成功.);
	 *		Result(1,删除用户消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(2,删除用户消息摘要信息失败[userOnlyId或msgType值为空]!);
	 */
	bean.Result deleteUserMsgDigestByUserOnlyIdAndMsgType (1:string maxCode,2:string minCode,3:string userOnlyId,4:string msgType)
	
	/**
	 * 删除某一用户的某一消息类别中某一个消息摘要信息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1906
	 * @param userOnlyId	     用户唯一ID
	 * @param msgType            消息类型
	 * @param msgId	 	 	 	 消息ID
	 * @return
	 *		Result(0,删除用户消息摘要信息成功.);
	 *      Result(1,删除用户消息摘要信息失败[minCode功能号不正确]!);
	 *		Result(1,删除用户消息摘要信息失败[userOnlyId或msgType值为空]!);
	 */
	bean.Result deleteUserMsgDigestByUserOnlyIdAndMsgTypeAndMsgId (1:string maxCode,2:string minCode,3:string userOnlyId,4:string msgType,5:string msgId)
	
	/**
	 * 更新某一用户的某一消息类别的消息摘要信息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1907
	 * @param userOnlyId     	 用户唯一ID
	 * @param msgType     	     消息类型
	 * @param msgId     	     消息Id
	 * @return
	 *		Result(0,更新用户消息摘要成功.);
	 *		Result(1,更新用户消息摘要失败[minCode功能号不正确]!);
	 *		Result(2,更新用户消息摘要失败[userOnlyId、msgType或msgId为空]!);
	 */
	bean.Result updateUserMsgDigest (1:string maxCode,2:string minCode,3:string userOnlyId,4:string msgType,5:string msgId)
	
	/**
	 * 查询某一用户的所有消息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1908
	 * @param userOnlyId	     用户唯一ID
	 * @return
	 *		map<string,list<string>>    某一用户的所有消息摘要
	 */
	map<string,list<string>> queryUserMsgDigestByUserOnlyId (1:string maxCode,2:string minCode,3:string userOnlyId)
	
	/**
	 * 查询某一用户的某一消息类别的所有消息
	 * @param maxCode		     缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：1909
	 * @param userOnlyId	     用户唯一ID
	 * @param msgType			 消息类别
	 * @return
	 *		list<string>         同一用户的某一类型的所有消息摘要
	 */
	list<string> queryDeviceMsgDigestByUserOnlyIdAndMsgType (1:string maxCode,2:string minCode,3:string userOnlyId,4:string msgType)
	
	/**
	 * 保存消息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2001
	 * @param MsgCache 	 	     消息
	 * @return
	 *		Result(0,保存消息成功.);
	 *		Result(1,保存消息失败[minCode功能号不正确]!);
	 *		Result(2,保存消息失败[MsgCache为空]!)
	 */
	 bean.Result saveMsg(1:string maxCode,2:string minCode,3:bean.MsgCache msgCache)
	
	/**
	 * 删除消息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2002
	 * @param msgId 	 		 消息ID
	 * @return
	 *		Result(0,删除消息成功.);
	 *		Result(1,删除消息失败[minCode功能号不正确]!);
	 *		Result(2,删除消息失败[msgId为空]!)
	 */
	bean.Result deleteMsg(1:string maxCode,2:string minCode,3:string msgId)
	
	/**
	 * 查询消息
	 * @param maxCode	缓存服务号：00FF0700
	 * @param minCode	缓存服务功能号：2003
	 * @param msgId 	消息缓存对象
	 * @return
	 *		MsgCache	消息缓存对象
	 */
	bean.MsgCache queryMsg(1:string maxCode,2:string minCode,3:string msgId)
	
	/**
	 * 保存TOKEN
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2101
	 * @param TokenCache 	 	 Token
	 * @return
	 *		Result(0,保存TOKEN成功.);
	 *		Result(1,保存TOKEN失败[minCode功能号不正确]!);
	 *		Result(2,保存TOKEN失败[TokenCache为空]!)
	 */
	 bean.Result saveToken(1:string maxCode,2:string minCode,3:bean.TokenCache tokenCache)
	 
	 /**
	 * 删除TOKEN
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2102
	 * @param appId 	 	     应用ID
	 * @return
	 *		Result(0,删除TOKEN成功.);
	 *		Result(1,删除TOKEN失败[minCode功能号不正确]!);
	 *		Result(2,删除TOKEN失败[appId为空]!)
	 *      Result(3,删除TOKEN失败[TOKEN不存在]!)
	 */
	 bean.Result deleteTokenByAppId(1:string maxCode,2:string minCode,3:string appId)
	 
	 /**
	 * 修改TOKEN
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2103
	 * @param appId 	 	     应用ID
	 * @return
	 *		Result(0,修改TOKEN成功.);
	 *		Result(1,修改TOKEN失败[minCode功能号不正确]!);
	 *		Result(2,修改TOKEN失败[appId为空]!)
	 *      Result(3,修改TOKEN失败[TOKEN不存在]!)
	 */
	 bean.Result updateTokenByAppId(1:string maxCode,2:string minCode,3:bean.TokenCache tokenCache)
	 
	 /**
	 * 查询TOKEN
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2104
	 * @param appId 	 	     应用ID
	 * @return
	 *		TokenCache           TokenCache对象
	 */
	 bean.TokenCache queryTokenByAppId(1:string maxCode,2:string minCode,3:string appId)
	
	 /**
	 * 保存浏览器会话信息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2201
	 * @param sId 	 	         "sId:"+sessionId
	 * @param HashMap            sessionId会话主要内容           
	 * @return
	 *		Result(0,保存session成功.);
	 *		Result(1,保存session失败[minCode功能号不正确]!);
	 *		Result(2,保存session失败[sId为空]!)
	 *		Result(3,保存session失败[hashMap为空]!)
	 */
	 bean.Result saveSession(1:string maxCode,2:string minCode,3:binary sId,4:map<binary,binary> hashMap)
	 
	 /**
	 * 保存浏览器单一会话信息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2202
	 * @param sId 	 	         "sId:"+sessionId
	 * @param key            	 session会话的key
	 * @param value              session会话的主要内容
	 * @return
	 *		Result(0,保存session成功.);
	 *		Result(1,保存session失败[minCode功能号不正确]!);
	 *		Result(2,保存session失败[sId为空]!)
	 *		Result(3,保存session失败[key或者value为空]!)
	 */
	 bean.Result saveSessionByDetail(1:string maxCode,2:string minCode,3:binary sId,4:binary key,5:binary value)
	 
	 /**
	 * 删除浏览器会话信息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2203
	 * @param sId 	 	         "sId:"+sessionId
	 * @return
	 *		Result(0,保存session成功.);
	 *		Result(1,保存session失败[minCode功能号不正确]!);
	 *		Result(2,保存session失败[sId为空]!)
	 */
	 bean.Result deleteSession(1:string maxCode,2:string minCode,3:binary sId)
	 
	 /**
	 * 删除浏览器会话信息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2204
	 * @param sId 	 	         "sId:"+sessionId
	 * @param key                单一session会话信息的key值
	 * @return
	 *		Result(0,保存session成功.);
	 *		Result(1,保存session失败[minCode功能号不正确]!);
	 *		Result(2,保存session失败[sId或key为空]!)
	 */
	 bean.Result deleteSessionByDetail(1:string maxCode,2:string minCode,3:binary sId,4:binary key)
	 
	 /**
	 * 修改浏览器单一会话信息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2205
	 * @param sId 	 	         "sId:"+sessionId
	 * @param key                单一session会话信息的key值
	 * @param value              单一session会话信息的value值
	 * @return
	 *		Result(0,保存session成功.);
	 *		Result(1,保存session失败[minCode功能号不正确]!);
	 *		Result(2,保存session失败[sId为空]!)
	 *      Result(3,保存session失败[key或者value为空]!)
	 */
	 bean.Result updataSessionByDetail(1:string maxCode,2:string minCode,3:binary sId,4:binary key,5:binary value)
	 
	 /**
	 * 修改浏览器会话信息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2206
	 * @param sId 	 	         "sId:"+sessionId
	 * @param HashMap            hashMap会话对象
	 * @return
	 *		Result(0,保存session成功.);
	 *		Result(1,保存session失败[minCode功能号不正确]!);
	 *		Result(2,保存session失败[sId为空]!)
	 *      Result(3,保存session失败[hashMap为空]!)
	 */
	 bean.Result updateSession(1:string maxCode,2:string minCode,3:binary sId,4:map<binary,binary> hashMap)
	 
	 /**
	 * 查询浏览器单一会话信息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2207
	 * @param sId 	 	         "sId:"+sessionId
	 * @param key                单一session会话信息的key值
	 * @return
	 *		byte[] value
	 */
	 binary querySessionByDetail(1:string maxCode,2:string minCode,3:binary sId,4:binary key)
	 
	 /**
	 * 查询浏览器会话信息
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：2208
	 * @param sId 	 	         "sId:"+sessionId
	 * @return
	 *		map<binary,binary>  hashMap
	 */
	 map<binary,binary> querySession(1:string maxCode,2:string minCode,3:binary sId)
	 
	 /**
	 * 通用接口:hash类型单字段增加或修改
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9100
	 * @param key 	 	         键
	 * @param field  	 	     字段
	 * @param value  	 	     字段值
	 * @return
	 *		Result(0,缓存操作hset成功.);
	 *		Result(21,缓存操作hset失败[传入参数maxCode为空]!)
	 *		Result(22,缓存操作hset失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作hset失败[传入参数key值为空]!)
	 *		Result(24,缓存操作hset失败[传入参数field值为空]!)
	 *		Result(25,缓存操作hset失败[传入参数value值为空]!)
	 *		Result(26,缓存操作hset失败[调用redis异常]!)
	 */
	 bean.Result commonHset(1:string maxCode,2:string minCode,3:string key,4:string field,5:string value)
	 
	 /**
	 * 通用接口:hash类型多字段增加或修改
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9101
	 * @param key 	 	         键
	 * @param hashMap  	 	         map存储格式为：<filed,value>  字段名和字段值
	 * @return
	 *		Result(0,缓存操作hmset成功.);
	 *		Result(21,缓存操作hmset失败[传入参数maxCode为空]!)
	 *		Result(22,缓存操作hmset失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作hmset失败[传入参数key值为空]!)
	 *		Result(24,缓存操作hmset失败[传入参数hashMap值为空]!)
	 *		Result(25,缓存操作hmset失败[调用redis异常]!)
	 */
	 bean.Result commonHmset(1:string maxCode,2:string minCode,3:string key,4:map<string,string> hashMap)
	 
	 /**
	 * 通用接口:hash类型新增
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9102
	 * @param key 	 	         键
	 * @param field  	 	     字段
	 * @param value  	 	     字段值
	 * @return
	 *		Result(0,缓存操作hsetnx成功.);
	 *		Result(21,缓存操作hsetnx失败[传入参数maxCode为空]!)
	 *		Result(22,缓存操作hsetnx失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作hsetnx失败[传入参数key值为空]!)
	 *		Result(24,缓存操作hsetnx失败[传入参数field值为空]!)
	 *		Result(25,缓存操作hsetnx失败[传入参数value值为空]!)
	 *		Result(26,缓存操作hsetnx失败[调用redis异常]!)
	 */
	 bean.Result commonHsetnx(1:string maxCode,2:string minCode,3:string key,4:string field,5:string value)
	 
	 /**
	 * 通用接口:hash类型删除
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9103
	 * @param key 	 	         键
	 * @param field  	 	     字段
	 * @return
	 *		Result(0,缓存操作hdel成功.);
	 *		Result(21,缓存操作hdel失败[传入参数maxCode为空]!)
	 *		Result(22,缓存操作hdel失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作hdel失败[传入参数key值为空]!)
	 *		Result(24,缓存操作hdel失败[传入参数field值为空]!)
	 *		Result(25,缓存操作hdel失败[调用redis异常]!)
	 */
	 bean.Result commonHdel(1:string maxCode,2:string minCode,3:string key,4:list<string> fields)
	 
	 /**
	 * 通用接口:hash类型批量查询
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9104
	 * @param keys 	 	         多个键
	 * @return
	 *		map<string,map<string,string>
	 */
	 map<string,map<string,string>> commonBatchHget(1:string maxCode,2:string minCode,3:list<string> keys)
	 
	 /**
	 * 通用接口:hash类型查询指定hash所有数据
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9105
	 * @param key 	 	         键
	 * @return
	 *		map<string,string>
	 */
	 map<string,string> commonHgetAll(1:string maxCode,2:string minCode,3:string key)
	 
	 /**
	 * 通用接口:hash类型查询字段对应的值
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9106
	 * @param key 	 	         键
	 * @param field 	 	     字段
	 * @return
	 *		string
	 */
	 string commonHget(1:string maxCode,2:string minCode,3:string key,4:string field)
	 
	 /**
	 * 通用接口:hash类型查询多个字段对应的值
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9107
	 * @param key 	 	         多个键
	 * @param fields 	 	     字段集合
	 * @return
	 *		list<string>
	 */
	 list<string> commonHmget(1:string maxCode,2:string minCode,3:string key,4:list<string> fields)
	 
	 /**
	 * 通用接口:hash类型字段模糊分页查询
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9108
	 * @param key 	 	         多个键
	 * @param cursor             起始位置(游标,从0开始)
	 * @param match              匹配规则表达式 匹配field  可选参数
	 * @param count              每次返回数据条数，期望值非实际值可选参数
	 * @return
	 *		map<string,map<string, string>>
	 */
	 map<string, string> commonHscan(1:string maxCode,2:string minCode,3:string key ,4:string cursor,5:string match,6:string count)
	 
	 /**
	 * 通用接口:hash类型查询hash所有字段值
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9109
	 * @param key 	 	         键
	 * @return
	 *		list<string>
	 */
	 list<string> commonHvals(1:string maxCode,2:string minCode,3:string key)
	 
	 /**
	 * 通用接口:hash类型查询字段是否存在
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9110
	 * @param key 	 	         键
	 * @param field              字段
	 * @return
	 *		bool
	 */
	 bool commonHexists(1:string maxCode,2:string minCode,3:string key,4:string field)
	 
	 /**
	 * 通用接口:hash类型统计字段数量
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9111
	 * @param key	 	         键
	 * @return
	 *		i64
	 */
	 i64 commonHlen(1:string maxCode,2:string minCode,3:string key)
	 
	 /**
	 * 通用接口:zset类型增加或更新
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9400
	 * @param key	 	         键
	 * @param hashMap	 	     hashMap存储格式为：<member,score>
	 * @param options	 	     操作类型(传入值必须为XX、NX或CH，具体解释见备注)  可选参数
	 * @return
	 *		Result(0,缓存操作zadd成功.);
	 *		Result(21,缓存操作zadd失败[传入参数maxCode为空]!)
	 *		Result(22,缓存操作zadd失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作zadd失败[传入参数key值为空]!)
	 *		Result(24,缓存操作zadd失败[传入参数hashMap值为空]!)
	 *		Result(25,缓存操作zadd失败[调用redis异常]!)
	 */
	 bean.Result commonZadd(1:string maxCode,2:string minCode,3:string key,4:map<string, double> hashMap,5:string options)
	 
	 /**
	 * 通用接口:zset类型删除成员
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9401
	 * @param key	 	         键
	 * @param members	 	     多个成员
	 * @return
	 *		Result(0,缓存操作zrem成功.);
	 *		Result(21,缓存操作zrem失败[传入参数maxCode为空]!)
	 *		Result(22,缓存操作zrem失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作zrem失败[传入参数key值为空]!)
	 *		Result(24,缓存操作zrem失败[传入参数members值为空]!)
	 *		Result(25,缓存操作zrem失败[调用redis异常]!)
	 */
	 bean.Result commonZrem(1:string maxCode,2:string minCode,3:string key,4:list<string> members)
	 
	 /**
	 * 通用接口:zset类型按指定分数范围删除成员
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9402
	 * @param key	 	         键
	 * @param min	 	         分数起始值
	 * @param max	 	         分数结束值
	 * @return
	 *		Result(0,缓存操作zremrangebyscore成功.);
	 *		Result(21,缓存操作zremrangebyscore失败[传入参数maxCode为空]!)
	 *		Result(22,缓存操作zremrangebyscore失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作zremrangebyscore失败[传入参数key值为空]!)
	 *		Result(24,缓存操作zremrangebyscore失败[传入参数min值为空]!)
	 *		Result(25,缓存操作zremrangebyscore失败[传入参数max值为空]!)
	 *		Result(26,缓存操作zremrangebyscore失败[调用redis异常]!)
	 */
	 bean.Result commonZremrangeByScore(1:string maxCode,2:string minCode,3:string key,4:double min,5:double max)
	 
	 /**
	 * 通用接口:zset类型按指定排名范围删除成员
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9403
	 * @param key	 	         键
	 * @param min	 	         分数起始值
	 * @param max	 	         分数结束值
	 * @return
	 *		Result(0,缓存操作zremrangebyrank成功.);
	 *	    Result(21,缓存操作zremrangebyrank失败[传入参数maxCode为空]!);
	 *		Result(22,缓存操作zremrangebyrank失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作zremrangebyrank失败[传入参数key值为空]!)
	 *		Result(24,缓存操作zremrangebyrank失败[传入参数min值为空]!)
	 *		Result(25,缓存操作zremrangebyrank失败[传入参数max值为空]!)
	 *		Result(26,缓存操作zremrangebyrank失败[调用redis异常]!)
	 */
	 bean.Result commonZremrangeByRank(1:string maxCode,2:string minCode,3:string key,4:i64 min,5:i64 max)
	 
	 /**
	 * 通用接口:zset类型按指定字典顺序范围删除成员
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9404
	 * @param key	 	         键
	 * @param min	 	         起始值
	 * @param max	 	         结束值
	 * @return
	 *		Result(0,缓存操作zremrangebylex成功.);
	 *		Result(21,缓存操作zremrangebylex失败[传入参数maxCode为空]!);
	 *		Result(22,缓存操作zremrangebylex失败[传入参数minCode为空]!)
	 *		Result(23,缓存操作zremrangebylex失败[传入参数key值为空]!)
	 *		Result(24,缓存操作zremrangebylex失败[传入参数min值为空]!)
	 *		Result(25,缓存操作zremrangebylex失败[传入参数max值为空]!)
	 *		Result(26,缓存操作zremrangebylex失败[调用redis异常]!)
	 *
	 */
	 bean.Result commonZremrangeByLex(1:string maxCode,2:string minCode,3:string key,4:string min,5:string max)
	 
	 /**
	 * 通用接口:zset类型统计成员个数
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9405
	 * @param key	 	         键
	 * @return
	 *		i64
	 */
	 i64 commonZcard(1:string maxCode,2:string minCode,3:string key)
	 
	 /**
	 * 通用接口:zset类型查询成员对应的值
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9406
	 * @param key	 	         键
	 * @return
	 *		double
	 */
	 double commonZscore(1:string maxCode,2:string minCode,3:string key,4:string member)
	 
	 /**
	 * 通用接口:zset类型查询指定成员的排名
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9407
	 * @param key	 	         键
	 * @param member	 	     成员
	 * @param sortType	 	     排序方式asc或desc
	 * @return
	 *		i64
	 */
	 i64 commonZrank(1:string maxCode,2:string minCode,3:string key,4:string member,5:string sortType)
	 
	 /**
	 * 通用接口:zset类型按指定分数范围分页查询成员
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9408
	 * @param key	 	         键
	 * @param min	 	         分数查询起始值
	 * @param max	 	         分数查询结束值
	 * @param sortType		     排序方式 asc 或 desc
	 * @param offset		     分页起始位置
	 * @param count		     	 输出个数
	 * @return
	 *		set<string>
	 */
	 set<string> commonPageZrangeByScore (1:string maxCode,2:string minCode,3:string key,4:double min,5:double max,6:string sortType,7:i32 offset,8:i32 count)
	 
	 /**
	 * 通用接口:zset类型按指定分数范围分页查询成员和分数
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9409
	 * @param key	 	         键
	 * @param min	 	         分数查询起始值
	 * @param end	 	         分数查询结束值
	 * @param sortType		     排序方式 asc 或 desc
	 * @param offset		     分页起始位置
	 * @param count		     	 输出个数
	 * @return
	 *		map<string,string>
	 */
	 map<string,string> commonPageZrangeByScoreWithScores(1:string maxCode,2:string minCode,3:string key,4:double min,5:double max,6:string sortType,7:i32 offset,8:i32 count)
	 
	 /**
	 * 通用接口:zset类型按指定分数范围查询成员和分数
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9410
	 * @param key	 	         键
	 * @param min	 	         分数查询起始值
	 * @param max	 	         分数查询结束值
	 * @param sortType		     排序方式 asc 或 desc
	 * @return
	 *		map<string,string>
	 */
	 map<string,string> commonZrangeByScore(1:string maxCode,2:string minCode,3:string key,4:double min,5:double max,6:string sortType)
	 
	 /**
	 * 通用接口:zset类型按指定分数范围查询成员
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9411
	 * @param key	 	         键
	 * @param min	 	         分数查询起始值
	 * @param max	 	         分数查询结束值
	 * @param sortType		     排序方式 asc 或 desc
	 * @return
	 *		set<string>
	 */
	 set<string> commonZrangeByScoreWithScores(1:string maxCode,2:string minCode,3:string key,4:double min,5:double max, 6:string sortType)
	 
	 /**
	 * 通用接口:zset类型按指定分数范围查询成员
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9412
	 * @param key	 	         键
	 * @param min 	 	     	 分数查询起始值
	 * @param max	 	         分数查询结束值
	 * @param sortType		     排序方式 asc 或 desc
	 * @return
	 *		set<string>
	 */
	 set<string> commonZrange(1:string maxCode,2:string minCode,3:string key,4:i64 min,5:i64 max,6:string sortType)
	 
	 /**
	 * 通用接口:zset类型加减成员对应的值
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9413
	 * @param key	 	         键
	 * @param score 	 	     值
	 * @param member	 	     成员
	 * @return
	 *		double
	 */
	 double commonZincrby(1:string maxCode,2:string minCode,3:string key,4:double sorce,5:string member)
	 
	 /**
	 * 通用接口:zset类型加减成员对应的值
	 * @param maxCode		 	 缓存服务号：00FF0700
	 * @param minCode		     缓存服务功能号：9414
	 * @param key	 	         键
	 * @param min 	 	         最小值
	 * @param max	 	         最大值
	 * @return
	 *		i64
	 */
	 i64 commonZcount(1:string maxCode,2:string minCode,3:string key,4:double min,5:double max)
}