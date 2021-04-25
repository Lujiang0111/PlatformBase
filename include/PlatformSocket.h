#ifndef PLATFORM_SOCKET_H_
#define PLATFORM_SOCKET_H_

#if defined (WIN32) || defined (_WINDLL)
#else
#include <netinet/in.h>
#endif

#include "PlatformBaseApi.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum PSocketInetType_
	{
		PS_INET_TYPE_UNKNOWN = 0,
		PS_INET_TYPE_IPV4,
		PS_INET_TYPE_IPV6,
	}EPSocketInetType;

	/************************************************************************
	*函数功能：	初始化套接字环境,win下调用WSAStartup使用v2.2库,linux下实际无操作.使用本模块
	*			时需要先调用此函数
	*返回值：	0成功,其它失败
	************************************************************************/
	PLATFORM_BASE_API int PSocketInitEnv();

	/************************************************************************
	*函数功能：	清理套接字环境,win下调用WSACleanup,linux下实际无操作
	*返回值：	0成功,其它失败
	************************************************************************/
	PLATFORM_BASE_API void PSocketCleanEnv();

	/************************************************************************
	*函数功能：	获取最后一次失败的信息
	*返回值：	最后一次失败的信息
	************************************************************************/
	PLATFORM_BASE_API int PSocketGetLastError();

	/************************************************************************
	*函数功能：	设置套按字的阻塞模式
	*函数参数：	fd			[IN]	需要处理的套按字
	*			mode		[IN]	0非阻塞模式,1阻塞模式
	*返回值：	0成功,其它失败
	*注意事项：	套接字默认为阻塞模式
	************************************************************************/
	PLATFORM_BASE_API int PSocketSetBlockMode(int fd, int mode);

	/************************************************************************
	*函数功能：	TCP套接字的keepalive设置
	*函数参数：	fd			[IN]	需要处理的套按字
	*			timeoutMs	[IN]	keepalive的超时时间(单位毫秒),0代表关闭,
	*								非0时最小时间为2S,少于2S按2S处理
	*返回值：	0成功,其它失败
	*注意事项：	默认套接字是不开启keepalive的
	************************************************************************/
	PLATFORM_BASE_API int PSocketSetTcpKeepalive(int fd, int timeoutMs);

	/************************************************************************
	*函数功能：	UDP套接字的ttl设置
	*函数参数：	fd			[IN]	需要处理的套按字
	*			ttl			[IN]	ttl值
	*返回值：	0成功,其它失败
	************************************************************************/
	PLATFORM_BASE_API int PSocketSetUdpTtl(int fd, int ttl);

	/************************************************************************
	*函数功能：	查询ip地址的ip类型
	*函数参数：	ip			[IN]	ip地址
	*返回值：	ip类型
	************************************************************************/
	PLATFORM_BASE_API EPSocketInetType PSocketGetInetType(const char *ip);

	/************************************************************************
	*函数功能：	查询ip地址是否为组播地址
	*函数参数：	ip			[IN]	ip地址
	*返回值：	0	是组播地址
				-1	不是组播地址
	************************************************************************/
	PLATFORM_BASE_API int PSocketIsMulticast(const char *ip);

	/************************************************************************
	*函数功能：	ip地址比较
	*输入参数:	lhs			[IN]	ip地址
				rhs			[IN]	ip地址
	*返回值：	-1	lhs < rhs
				0	lhs = rhs
				1	lhs > rhs
				-2	无法比较
	************************************************************************/
	PLATFORM_BASE_API int PSocketIpCompare(const char *lhs, const char *rhs);

	/************************************************************************/
	/* 以下接口对sockaddr进行简单封装，以实现对IPV4和IPV6的同时支持			    */
	/************************************************************************/
	typedef void *PSockAddrHandle;

	/************************************************************************
	*函数功能：	通过域名创建PSockAddrHandle实例
	*输入参数:	host		[IN]	ip地址或者域名
				port		[IN]	端口
	*返回值：	实例句柄
	*注意事项：	不再使用时调用PSockAddrDestroy销毁实例
	************************************************************************/
	PLATFORM_BASE_API PSockAddrHandle PSockAddrCreate(const char *host, uint16_t port);

	/************************************************************************
	*函数功能：	通过IP地址创建本地PSockAddrHandle实例
	*输入参数:	ifIp		[IN]	网卡ip地址
				port		[IN]	端口
	*返回值：	实例句柄
	*注意事项：	创建本地地址实例时请调用此接口，接口会自动获取网卡信息
				不再使用时调用PSockAddrDestroy销毁实例
	************************************************************************/
	PLATFORM_BASE_API PSockAddrHandle PSockAddrCreateLocal(const char *ifIp, uint16_t port);

	/************************************************************************
	*函数功能：	销毁PSockAddrHandle实例
	*输入参数:	pHdl		[IN]	实例句柄指针
	*注意事项：	*pHdl会置NULL
	************************************************************************/
	PLATFORM_BASE_API void PSockAddrDestroy(PSockAddrHandle *pHdl);

	/************************************************************************
	*函数功能：	返回PSockAddrHandle实例的ip类型
	*函数参数：	ip			[IN]	ip地址
	*返回值：	ip类型
	************************************************************************/
	PLATFORM_BASE_API EPSocketInetType PSockAddrGetInetType(PSockAddrHandle hdl);

	/************************************************************************
	*函数功能：	返回PSockAddrHandle实例的原生struct sockaddr *结构
	*输入参数:	hdl			[IN]	实例句柄
	*返回值：	原生struct sockaddr *结构
	***********************************************************************/
	PLATFORM_BASE_API const struct sockaddr *PSockAddrGetNative(PSockAddrHandle hdl);

	/************************************************************************
	*函数功能：	返回PSockAddrHandle实例的ip地址
	*输入参数:	hdl			[IN]	实例句柄
	*返回值：	ip地址
	************************************************************************/
	PLATFORM_BASE_API const char *PSockAddrGetIp(PSockAddrHandle hdl);

	/************************************************************************
	*函数功能：	返回PSockAddrHandle实例的端口
	*输入参数:	hdl			[IN]	实例句柄
	*返回值：	端口
	************************************************************************/
	PLATFORM_BASE_API uint16_t PSockAddrGetPort(PSockAddrHandle hdl);

	/************************************************************************
	*函数功能：	返回本地PSockAddrHandle实例的网卡名称
	*输入参数:	hdl			[IN]	实例句柄
	*返回值：	网卡名称
	************************************************************************/
	PLATFORM_BASE_API const char *PSockAddrGetIfName(PSockAddrHandle hdl);

	/************************************************************************
	*函数功能：	socket加入组播组
	*输入参数:	hdl			[IN]	实例句柄
				groupAddr	[IN]	组播地址
				localAddr	[IN]	网卡地址
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrJoinMulticastGroup(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr);

	/************************************************************************
	*函数功能：	socket离开组播组
	*输入参数:	hdl			[IN]	实例句柄
				groupAddr	[IN]	组播地址
				localAddr	[IN]	网卡地址
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrLeaveMulticastGroup(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr);

	/************************************************************************
	*函数功能：	组播组添加源
	*输入参数:	hdl			[IN]	实例句柄
				groupAddr	[IN]	组播地址
				localAddr	[IN]	网卡地址
				sourceAddr	[IN]	源地址
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrAddSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr);

	/************************************************************************
	*函数功能：	组播组删除源
	*输入参数:	hdl			[IN]	实例句柄
				groupAddr	[IN]	组播地址
				localAddr	[IN]	网卡地址
				sourceAddr	[IN]	源地址
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrDropSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr);

	/************************************************************************
	*函数功能：	组播组屏蔽源
	*输入参数:	hdl			[IN]	实例句柄
				groupAddr	[IN]	组播地址
				localAddr	[IN]	网卡地址
				sourceAddr	[IN]	源地址
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrBlockSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr);

	/************************************************************************
	*函数功能：	组播组解除屏蔽源
	*输入参数:	hdl			[IN]	实例句柄
				groupAddr	[IN]	组播地址
				localAddr	[IN]	网卡地址
				sourceAddr	[IN]	源地址
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrUnblockSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr);

	/************************************************************************
	*函数功能：	ip地址+1，端口不变
	*输入参数:	hdl			[IN]	实例句柄
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrIncrease(PSockAddrHandle hdl);

	/************************************************************************
	*函数功能：	ip地址-1，端口不变
	*输入参数:	hdl			[IN]	实例句柄
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrDecrease(PSockAddrHandle hdl);

	/************************************************************************
	*函数功能：	ip地址比较
	*输入参数:	lhs			[IN]	左实例句柄
				rhs			[IN]	右实例句柄
	*返回值：	-1	lhs < rhs
				0	lhs = rhs
				1	lhs > rhs
				-2	无法比较
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrCompare(PSockAddrHandle lhs, PSockAddrHandle rhs);

	/************************************************************************
	*函数功能：	socket bind封装
	*输入参数:	fd			[IN]	socket
				serverHdl	[IN]	目标地址句柄
				localHdl	[IN]	本地地址句柄，用于接受组播时指定网卡，可以为NULL
	*返回值：	0	成功
				-1	失败
	*注意事项：	内部会自动调用reuse addr与reuse port
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrBind(int fd, PSockAddrHandle serverHdl, PSockAddrHandle localHdl);

	/************************************************************************
	*函数功能：	socket accept封装
	*输入参数:	fd			[IN]	socket
				pHdl		[OUT]	accept获取的实例句柄地址
	*返回值：	>0	成功，值为accept获取的fd
				-1	失败
	*注意事项：	若(*hdl)原来有数据，则该数据会被替换
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrAccept(int fd, PSockAddrHandle *pHdl);

	/************************************************************************
	*函数功能：	socket connect封装
	*输入参数:	fd			[IN]	socket
				hdl			[IN]	实例句柄
	*返回值：	0	成功
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrConnect(int fd, PSockAddrHandle hdl);

	/************************************************************************
	*函数功能：	socket recvfrom封装
	*输入参数:	fd			[IN]	socket
				pHdl		[OUT]	recvform获取的实例句柄地址
				buf			[INOUT]	写入地址
				len			[IN]	最大写入长度
	*返回值：	>0	成功，值为接收字节长度
				=0	表示对端已关闭连接
				-1	失败
	*注意事项：	若(*hdl)原来有数据，则该数据会被替换
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrRecvfrom(int fd, PSockAddrHandle *pHdl, char *buf, int len);

	/************************************************************************
	*函数功能：	socket sendto封装
	*输入参数:	fd			[IN]	socket
				hdl			[IN]	实例句柄
				buf			[IN]	发送地址
				len			[IN]	发送长度
	*返回值：	>=0	成功，值为发送字节长度
				-1	失败
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrSendto(int fd, PSockAddrHandle hdl, const char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_SOCKET_H_