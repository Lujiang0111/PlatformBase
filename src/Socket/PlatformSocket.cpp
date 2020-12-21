#include <string.h>
#include <errno.h>
#include "PlatformSocket.h"
#include "Base/IPlatformBaseApi.h"

#if defined (WIN32) || defined (_WINDLL)
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

int PSocketInitEnv()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		PBLogOut(PL_LEVEL_ERROR, "WSAStartup failed with error: %d\n", err);
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		PBLogOut(PL_LEVEL_ERROR, "Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		return -1;
	}

	return 0;
}

void PSocketCleanEnv()
{
	WSACleanup();
}

int PSocketGetLastError()
{
	return WSAGetLastError();
}

int PSocketSetBlockMode(int fd, int mode)
{
	u_long iMode = (0 == mode) ? 1 : 0;
	if (ioctlsocket(fd, FIONBIO, &iMode) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "Unable to set block mode[%d], error = %d", mode, PSocketGetLastError());
		return -1;
	}
	return 0;
}

int PSocketSetTcpKeepalive(int fd, int timeoutMs)
{
	tcp_keepalive alive;
	if (0 == timeoutMs)
	{
		alive.onoff = 0;
	}
	else
	{
		if (timeoutMs < 2000)
		{
			timeoutMs = 2000;
		}
		alive.keepalivetime = timeoutMs / 2;			// 一半超时时间后进入探测
		alive.keepaliveinterval = timeoutMs / 20;		// 系统默认探测次数为10次,所以时间为一半超时时间的1/10
		alive.onoff = 1;
	}

	DWORD ulBytesReturn = 0;
	if (WSAIoctl(fd, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),
		NULL, 0, &ulBytesReturn, NULL, NULL) == SOCKET_ERROR)
	{
		PBLogOut(PL_LEVEL_ERROR, "WSAIoctl failed with error:%d", PSocketGetLastError());
		return -1;
	}
	return 0;
}

#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int PSocketInitEnv()
{
	return 0;
}

void PSocketCleanEnv()
{
}

int PSocketGetLastError()
{
	return errno;
}

int PSocketSetBlockMode(int fd, int mode)
{
	if (fcntl(fd, F_GETFL, NULL) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "Unable to set block mode[%d], error:%d", mode, PSocketGetLastError());
		return -1;
	}

	int arg = 0;
	if (0 == mode)
	{
		arg |= O_NONBLOCK;
	}
	else
	{
		arg &= ~O_NONBLOCK;
	}
	if (fcntl(fd, F_SETFL, arg) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "Unable to set block mode[%d], error:%d", mode, PSocketGetLastError());
		return -1;
	}

	return 0;
}

int PSocketSetTcpKeepalive(int fd, int timeoutMs)
{
	if (0 == timeoutMs)
	{
		int value = 0;
		if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&value, sizeof(int)) != 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "Unable to disable SO_KEEPALIVE, error:%d", PSocketGetLastError());
			return -1;
		}
	}
	else
	{
		int value = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&value, sizeof(int)) != 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "Unable to enable SO_KEEPALIVE, error:%d", PSocketGetLastError());
			return -1;
		}

		if (timeoutMs < 2000)
		{
			timeoutMs = 2000;
		}

		//一半时间后进入keepalive状态
		value = timeoutMs / 2000;
		if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (const char *)&value, sizeof(int)) != 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "Unable to set TCP_KEEPIDLE, use system defalut, error:%d", PSocketGetLastError());
		}

		value = 1;
		if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (const char *)&value, sizeof(int)) != 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "Unable to set TCP_KEEPINTVL, use system defalut, error:%d", PSocketGetLastError());
		}

		value = timeoutMs / 2000;
		if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (const char *)&value, sizeof(int)) != 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "Unable to set TCP_KEEPCNT, use system defalut, error:%d", PSocketGetLastError());
		}

		return 0;
	}
	return 0;
}

#endif

int PSocketSetUdpTtl(int fd, int ttl)
{
	int ret = -1;

	// for ipv4
	if (0 == setsockopt(fd, IPPROTO_IP, IP_TTL, (const char *)&ttl, sizeof(ttl)))
	{
		ret = 0;
	}

	if (0 == setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof(ttl)))
	{
		ret = 0;
	}

	// for ipv6
	if (0 == setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (const char *)&ttl, sizeof(ttl)))
	{
		ret = 0;
	}

	if (0 == setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const char *)&ttl, sizeof(ttl)))
	{
		ret = 0;
	}

	return ret;
}

EPSocketInetType PSocketGetInetType(const char *ip)
{
	// Ipv6为128位地址，需要16字节大小储存
	uint8_t buf[16] = { 0 };

	if (inet_pton(AF_INET, ip, buf) > 0)
	{
		return PS_INET_TYPE_IPV4;
	}
	else if (inet_pton(AF_INET6, ip, buf) > 0)
	{
		return PS_INET_TYPE_IPV6;
	}

	return PS_INET_TYPE_UNKNOWN;
}

int PSocketIsMulticast(const char *ip)
{
	EPSocketInetType inetType = PSocketGetInetType(ip);
	if (PS_INET_TYPE_IPV4 == inetType)		// ipv4
	{
		return IN_MULTICAST(ntohl(inet_addr(ip)));
	}
	else if (PS_INET_TYPE_IPV6 == inetType)	// ipv6
	{
		unsigned char buf[16];
		inet_pton(AF_INET6, ip, buf);

		struct in6_addr addr_6;
		memcpy(addr_6.s6_addr, buf, sizeof(buf));
		return IN6_IS_ADDR_MULTICAST(&(addr_6));
	}

	PBLogOut(PL_LEVEL_ERROR, "unidentified ip type!");
	return -1;
}

int PSocketIpCompare(const char *lhs, const char *rhs)
{
	PSockAddrHandle lHdl = PSockAddrCreate(lhs, 0);
	PSockAddrHandle rHdl = PSockAddrCreate(rhs, 0);
	int ret = PSockAddrCompare(lHdl, rHdl);
	PSockAddrDestroy(&lHdl);
	PSockAddrDestroy(&rHdl);
	return ret;
}