#ifndef I_PLATFORM_SOCK_ADDR_IPV6_H_
#define I_PLATFORM_SOCK_ADDR_IPV6_H_

#include "Socket/IPlatformSockAddrBase.h"

class PSockAddrIpv6 : public PSockAddrBase
{
public:
	PSockAddrIpv6(const char *ip, uint16_t port, bool isLocal);
	PSockAddrIpv6(const struct sockaddr_in6 *pAddr, bool isLocal);
	virtual ~PSockAddrIpv6();

	virtual bool Increase();
	virtual bool Decrease();
	virtual int Compare(PSockAddrBase *rhs);

	virtual const struct sockaddr *GetNativeAddr() const;

	// for multicast group
	virtual bool JoinGroup(int fd, PSockAddrBase *localAddr);
	virtual bool LeaveGroup(int fd, PSockAddrBase *localAddr);
	virtual bool AddSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr);
	virtual bool DropSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr);
	virtual bool BlockSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr);
	virtual bool UnblockSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr);

	virtual bool Bind(int fd, PSockAddrBase *localAddr);
	virtual bool Connect(int fd);
	virtual int Sendto(int fd, const char *buf, int len);

private:
	PSockAddrIpv6() = delete;

	void SetLocalArgs();

private:
	struct sockaddr_in6 _addr;
	int _scopeId;
};

#endif // !I_PLATFORM_SOCK_ADDR_IPV6_H_