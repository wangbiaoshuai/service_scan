/**
 * <B>说 明</B>:缓存服务Thrift文件-结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月21日 上午11:09:58
 */
namespace java com.vrv.cems.service.base.bean.cache
namespace cpp com.vrv.cems.service.base.bean.cache
 
/**
 * <B>说 明</B>:返回结果结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct Result{
	/**
	 * 结果编码(默认值0，代表成功)
	 */
	1:optional i32 code=0;
	/**
	 * 结果描述信息
	 */
	2:optional string info="成功.";
 }
 
 /**
 * <B>说 明</B>:设备信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct DeviceCache{
	/**
	 * ID
	 */
	1:string id;
	/**
	 * 设备唯一ID
	 */
	2:string devOnlyId;
	/**
	 * IP地址
	 */
	3:string ip;
	/**
	 * IP类型
	 */
	4:string ipType;
	/**
	 * IP数值
	 */
	5:string ipNumber;
	/**
	 * MAC地址
	 */
	6:string mac;
	/**
	 * 操作系统ID
	 */
	7:string osId;
	/**
	 * 设备类型ID
	 */
	8:string deviceTypeId;
	/**
	 * CEMS客户端安装时间
	 */
	9:string registerTime;
	/**
	 * CEMS客户端唯一ID
	 */
	10:string clientId;
	/**
	 * CEMS客户端名称
	 */
	11:string clientName;
	/**
	 * CEMS客户端签名
	 */
	12:string clientSign;
	/**
	 * CEMS客户端注册程序版本号
	 */
	13:string clientVersion;
	/**
	 * CEMS客户端核心模块版本
	 */
	14:string softVersion;
	/**
	 * 所属交换机IP
	 */
	15:string switchIP
	/**
	 * 所属交换机端口
	 */
	16:string switchPort
	/**
	 * 是否是多操作系统(1-单操作系统,>=2-多操作系统)
	 */
	17:i32 isMutiOs
	/**
	 * 是否是虚拟机(0-不是,1-是)
	 */
	18:i32 isVm
	/**
	 * 客户端通讯ip
	 */
	19:string communicateIP
	/**
	 * 路由IP
	 */
	20:string routeIp
	/**
	 * 注册状态(0-注册,1-卸载)
	 */
	21:i32 regState	
	/**
	 * 保护状态(0-未保护,1-保护)
	 */
	22:i32 protectState	
	/**
	 * 漫游状态(0-未漫游,1-漫游)
	 */
	23:i32 roamState	
	/**
	 * 删除状态(0-正常数据,1-删除)
	 */
	24:i32 deleteState	
	/**
	 * 注册人Id(userOnlyId)
	 */
	25:string userOnlyId
	/**
	 * 所属组织机构ID
	 */
	26:string organizationId
	/**
	 * 子网掩码
	 */
	27:string mask
 }
 
 /**
 * <B>说 明</B>:设备在线情况结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct DeviceOnlineCache{
	/**
	 * 设备唯一ID
	 */
	1:string devOnlyId;
	/**
	 * 设备登陆时间
	 */
	2:string loginTime;
	/**
	 * 设备最近一次心跳时间(TCP)
	 */
	3:string activeTime;
	/**
	 * 设备登陆会话ID
	 */
	4:string sessionId;
	/**
	 * 设备最近一次心跳时间(UDP)
	 */
	5:string udpActiveTime;
	/**
	 * 设备UDP心跳路由IP
	 */
	6:string routeIp;
	/**
	 * 设备UDP心跳路由端口
	 */
	7:string udpPort;
	/**
	 * 设备连接UDP服务IP地址
	 */
	8:string udpServiceIp;
	/**
	 * 设备连接UDP服务端口
	 */
	9:string udpServicePort;
 }
 
 /**
 * <B>说 明</B>:设备会话密钥结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct DeviceKeyCache{
	/**
	 * 加密密钥类型
	 */
	1:string keyType;
	/**
	 * 密码
	 */
	2:string password;
	/**
	 * 设备唯一ID
	 */
	3:string devOnlyId;
	/**
	 * 会话ID
	 */
	4:string sessionId;
	/**
	 * 加密偏移向量
	 */
	5:string offsetVector;
 }
 /**
 * <B>说 明</B>:设备产品信息(已安装)结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct ProductInfoOld{
	/**
	 * 产品升级包类型(0-基础升级包,1-定制升级包)
	 */
	1:i32 type,
	/**
	 * 产品名称(升级包名称)
	 */
    2:string productName,
	/**
	 * 产品版本(升级包版本)
	 */
    3:string version,
	/**
	 * 产品安装包(升级包)签名信息
	 */
    4:string productSign,
	/**
	 * 产品在设备上的安装时间
	 */
    5:string installTime
 }
 
 /**
 * <B>说 明</B>:设备产品信息(应安装)结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct ProductInfoNew{
	/**
	 * 产品升级包类型(0-基础升级包,1-定制升级包)
	 */
	1:i32 type,
	/**
	 * 产品名称(升级包名称)
	 */
    2:string productName,
	/**
	 * 产品版本(升级包版本)
	 */
    3:string version,
	/**
	 * 产品升级包ID(对应缓存结构14中cUpgradePackId)
	 */
    4:string cUpgradePackId
 }
 
 /**
 * <B>说 明</B>:设备产品信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct DeviceProduct{
	/**
	 * 设备操作系统类型
	 */
	1:string osType;
	/**
	 * 产品类型
	 */
	2:string productType;
	/**
	 * 设备已安装产品集
	 */
	3:list<ProductInfoOld> productInfoOldList;
	/**
	 * 设备应安装产品集
	 */
	4:list<ProductInfoNew> productInfoNewList;
 }
 
 
 /**
 * <B>说 明</B>:设备执行策略信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct DevicePolicyCache{
	/**
	 * 设备唯一ID
	 */
	1:string devOnlyId;
	/**
	 * 策略总版本(具体计算格式详见《VRV-VDP-064-缓存数据结构设计-CEMS.doc》文档中“6.设备执行策略信息”)
	 */
	2:string policyVersion
	/**
	 * 策略ID，多个以","分开
	 */
	3:string policyIds
 }
 
 
 /**
 * <B>说 明</B>:用户信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct UserCache{
	/**
	 * ID
	 */
	1:string id;
	/**
	 * 用户唯一ID
	 */
	2:string userOnlyId;
	/**
	 * 帐号
	 */
	3:string account;
	/**
	 * 密码
	 */
	4:string password;
	/**
	 * 账号类型
	 */
	5:string accountTypeId;
	/**
	 * 组织机构ID
	 */
	6:string organizationId;
	/**
	 * 注册时间
	 */
	7:string registeTime;
	/**
	 * 最后登录时间
	 */
	8:string lastLoginTime;
	/**
	 * 审核状态
	 */
	9:i32 approvalState;
	/**
	 * 用户类型
	 */
	10:i32 type;
 }
 
 /**
 * <B>说 明</B>:用户在线情况结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct UserOnlineCache{
	/**
	 * 用户唯一ID
	 */
	1:string userOnlyId;
	/**
	 * 用户账号
	 */
	2:string account;
	/**
	 * 设备唯一ID
	 */
	3:string devOnlyId;
	/**
	 * 最后登录时间
	 */
	4:string loginTime;
	/**
	 * 登录操作系统的帐号
	 */
	5:string loginAccount
	/**
	 * 用户设备联合ID
	 * userDevId=userOnlyId+@+devOnlyId
	 */
	6:string userDevId	
 }
 
 
 /**
 * <B>说 明</B>:用户执行策略信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct UserPolicyCache{
	/**
	 * 用户唯一ID
	 */
	1:string userOnlyId
	/**
	 * 策略总版本(具体计算格式详见《VRV-VDP-064-缓存数据结构设计-CEMS.doc》文档中“9.设备执行策略信息”)
	 */
	2:string policyVersion
	/**
	 * 策略ID，多个以","分开
	 */
	3:string policyIds
 }
 
 /**
 * <B>说 明</B>:产品安装包信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct CInstallPackCache{
	/**
	 * 安装包ID
	 */
	1:string cInstallPackId;
	/**
	 * 安装包名称
	 */
	2:string name;
	/**
	 * 安装包版本
	 */
	3:string version;
	/**
	 * 安装包大小
	 */
	4:i32	size;
	/**
	 * 安装包在FASTDFS上的路径
	 */
	5:string path;
	/**
	 * 安装包操作系统类型
	 */
	6:string osType;
	/**
	 * 安装包产品类型
	 */
	7:string productType;
	/**
	 * 安装包发布时间
	 */
	8:string pubTime;
	/**
	 * 安装包CRC
	 */
	9:string crc; 
 }
 
/**
 * <B>说 明</B>:策略信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct PolicyCache{
	/**
	 * ID
	 */
	1:string id;
	/**
	 * 策略执行对象类型
	 */
	2:string objType;
	/**
	 * 名称
	 */
	3:string name;
	/**
	 * 内容
	 */
	4:string content;
	/**
	 * CRC值
	 */
	5:string crc;
	/**
	 * 策略content xml内容中policy表中的crc的属性值
	 */
	6:string contentCRC;
	/**
	 * 组织机构ID
	 */
	7:string organizationId;
	/**
	 * 下发时间
	 */
	8:string publishTime;
	/**
	 * 策略模板ID
	 */
	9:string policyTemplateId;
	/**
	 * 策略对象
	 */
	10:string obj;
	/**
	 * 策略对象CRC
	 */
	11:string objCRC;
 }
 
/**
 * <B>说 明</B>:产品升级包信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct CUpgradePackCache{
	/**
	 * 升级包ID
	 */
	1:string cUpgradePackId;
	/**
	 * 升级包名称
	 */
	2:string name;
	/**
	 * 升级包版本
	 */
	3:string version;
	/**
	 * 升级包大小
	 */
	4:i32	size;
	/**
	 * 升级包在FASTDFS上的路径
	 */
	5:string path;
	/**
	 * 升级包操作系统类型
	 */
	6:string osType;
	/**
	 * 升级包产品类型
	 */
	7:string productType;
	/**
	 * 升级包发布时间
	 */
	8:string pubTime;
	/**
	 * 升级包CRC
	 */
	9:string crc;
	/**
	 * 升级包类型：0-基础升级包，1-定制升级包
	 */
	10:i32 type;
 }
  
/**
 * <B>说 明</B>:敏感规则库信息结构体
 * 
 * @author 作 者 名：高林武<br/>
 *         E-mail ：linwu_gao@163.com
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2015年4月20日 上午10:09:58
 */
 struct SensitiveCache{
	/**
	 * ID
	 */
	1:string id;
	/**
	 * 文件名
	 */
	2:string name;
	/**
	 * 文件路径
	 */
	3:string path;
	/**
	 * 文件CRC值
	 */
	4:string crc;
	/**
	 * 文件创建时间
	 */
	5:string createTime;
	/**
	 * 文件最后修改时间
	 */
	6:string lastUpdateTime;
 }
 
 /**
 * <B>说 明</B>:消息结构体
 * 
 * @author 作 者 名：拜山峰<br/>
 *         E-mail ：baishanfeng@vrvmail.com.cn
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2016年11月1日 下午16:12:20
 */
 struct MsgCache{
	/**
	 * 消息ID
	 */
	1:string msgId;
	/**
	 * 消息类型
	 */
	2:string msgType;
	/**
	 * 消息内容
	 */
	3:string msgData;
	/**
	 * 消息产生时间
	 */
	4:string createTime;
 }
 
 /**
 * <B>说 明</B>:TOKEN结构体
 * 
 * @author 作 者 名：拜山峰<br/>
 *         E-mail ：baishanfeng@vrvmail.com.cn
 * 
 * @version 版 本 号：V1.0.<br/>
 *          创建时间：2016年12月21日 上午11:44:20
 */
 struct TokenCache{
	/**
	 * Token主键
	 */
	1:string token;
	/**
	 * 应用ID
	 */
	2:string appId;
	/**
	 * Token创建时间
	 */
	3:string createTime;
	/**
	 * Token过期时间
	 */
	4:string expiredTime;
	/**
	 * Token刷新时间
	 */
	5:string refreshTime;
 }
 