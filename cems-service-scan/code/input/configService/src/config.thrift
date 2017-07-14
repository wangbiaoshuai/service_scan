include "vrv.thrift"
namespace java com.vrv.im.service
namespace cpp com.vrv.im.service

struct ServiceConfigBean{
   /*服务名或者服务编码*/
   1:string serviceID;
   /*服务版本*/
   2:string version;
   /*服务状态属性 1:无状态 2:有状态*/
   3:optional byte property;
   /*服务IP地址*/
   4:string ip;
   /*服务端口 */
   5:i32 port;
   /*服务策略*/
   6:optional string tactics;
   /*服务名*/
   7:optional string name;
   /*服务描述*/
   8:optional string description;
   /*服务安装路径*/
   9:optional string location;
   /*服务安装时间*/
   10:optional i64 installTime;
   /*服务区域ID*/
   11:string orgID;
   /*服务器ID*/
   12:optional string serverID;
   /*服务失效间隔时长 单位毫秒*/
   13:optional i64 invalidTime;
   /*服务实例ID*/
   14:string SSID;
}

struct ServiceConfigResult{
    /*服务唯一标识*/
    1:string ID;
     /*服务主备属性1:主机 2:备机 4:没有服务器信息*/
    2:byte  property;
}

service ConfigService  extends vrv.VRVService {
  /**
   *********************************注册服务方法******************************************
   *各服务初次启动，注册服务信息到配置服务（后续每隔1秒维持心跳）。
   *参数：ServiceConfigBean：服务配置信息
   *返回：ServiceConfigResult
   *判断有没有该服务实例，如果没有存储之，返回服务信息的主备属性和服务唯一标识
   *如果有踢掉原来的服务信息，再在相同的服务中决定主备属性,返回服务信息的主备属性和服务唯一标识
   *
   */
   ServiceConfigResult registerService(1:ServiceConfigBean config)  
  /**
   *********************************服务心跳******************************************
   *启动服务后，维持心跳，标识服务可用
   *参数：服务唯一标识
   *返回：byte，1:主机 2:备机 3:重新注册 5:被踢
   *                 根据参数检测服务，判断服务列表是否有没有该实例服务
   *                   |                                                            |
   *               有|                                                        无|
   *        返回主备属性或者是否被踢           返回提醒重新注册
   *---------------------------------------------------------------------------------------
   *如果返回提醒重新注册，服务那边就需要重新调用注册方法
   *如果返回被踢,停止服务
   *
   */
   byte serviceHeart(1:string ID)

  /**
   ****************************提供给voa获取正常状态服务************************************
   *voa加载服务使用，定时每隔1秒调用一次。
   *参数：无
   *返回：无
   *返回服务信息列表  过滤被踢服务
   */
   list<ServiceConfigBean> loadServices()
  /**
   ****************************提供给路由服务器查询聊天服务器用************************************
   *服务查询，定时每隔1秒调用一次。
   *参数：无
   *返回：无
   *返回服务信息列表  过滤被踢服务
   */
   list<ServiceConfigBean> queryService(1:string serviceID,2:string version)

}
