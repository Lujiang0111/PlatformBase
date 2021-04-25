#ifndef I_PLATFORM_SOCK_ADDR_IPV4_H_
#define I_PLATFORM_SOCK_ADDR_IPV4_H_

#include "Socket/IPlatformSockAddrBase.h"

class PSockAddrIpv4 : public PSockAddrBase
{
public:
	PSockAddrIpv4(const char *ip, uint16_t port, bool isLocal);
	PSockAddrIpv4(const struct sockaddr_in *pAddr, bool isLocal);
	virtual ~PSockAddrIpv4();

	virtual bool Increase();
	virtual bool Decrease();
	virtual int Compare(PSockAddrBase *rhs);

	virtual EPSocketInetType GetInetType() const;
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
	PSockAddrIpv4() = delete;

	void SetLocalArgs();

private:
	struct sockaddr_in _addr;
};

#endif // !I_PLATFORM_SOCK_ADDR_IPV4_H_