#ifndef I_PLATFORM_SOCK_ADDR_BASE_H_
#define I_PLATFORM_SOCK_ADDR_BASE_H_

#if defined (WIN32) || defined (_WINDLL)
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif

#include <string>
#include "Base/IPlatformBaseApi.h"
#include "PlatformSocket.h"

class PSockAddrBase
{
public:
	PSockAddrBase();
	virtual ~PSockAddrBase();

	virtual bool Increase() = 0;
	virtual bool Decrease() = 0;
	virtual int Compare(PSockAddrBase *rhs) = 0;

	virtual EPSocketInetType GetInetType() const = 0;
	virtual const struct sockaddr *GetNativeAddr() const = 0;

	// for multicast group
	virtual bool JoinGroup(int fd, PSockAddrBase *localAddr) = 0;
	virtual bool LeaveGroup(int fd, PSockAddrBase *localAddr) = 0;
	virtual bool AddSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr) = 0;
	virtual bool DropSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr) = 0;
	virtual bool BlockSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr) = 0;
	virtual bool UnblockSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr) = 0;

	virtual bool Bind(int fd, PSockAddrBase *localAddr) = 0;
	virtual bool Connect(int fd) = 0;
	virtual int Sendto(int fd, const char *buf, int len) = 0;

	const std::string &GetIp() const;
	uint16_t GetPort() const;
	const std::string &GetIfName() const;

protected:
	std::string _ip;
	uint16_t _port;
	std::string _ifName;
	int _ifNum;
};

#endif // !I_PLATFORM_SOCK_ADDR_BASE_H_