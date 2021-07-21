#include <string.h>
#include "Socket/IPlatformSockAddrIpv4.h"
#include "Socket/IPlatformSockAddrIpv6.h"

PSockAddrHandle PSockAddrCreate(const char *host, uint16_t port)
{
	PSockAddrBase *h = NULL;

	EPSocketInetType inetType = PSocketGetInetType(host);
	switch (inetType)
	{
	case PS_INET_TYPE_IPV4:
		h = new PSockAddrIpv4(host, port, false);
		break;
	case PS_INET_TYPE_IPV6:
		h = new PSockAddrIpv6(host, port, false);
		break;
	case PS_INET_TYPE_UNKNOWN:
	{
		char sPort[16] = { 0 };
		sprintf(sPort, "%hu", port);
		struct addrinfo hints = { 0 };

		struct addrinfo *res = NULL;
		int ret = 0;
		while (EAI_AGAIN == (ret = getaddrinfo(host, sPort, &hints, &res)));
		if (0 != ret)
		{
			PBLogOut(PL_LEVEL_ERROR, "getaddrinfo fail! host=%s, port=%hu, error=%d", host, port, PSocketGetLastError());
			break;
		}

		for (struct addrinfo *nRes = res; res; res = res->ai_next)
		{
			if (AF_INET == res->ai_family)
			{
				h = new PSockAddrIpv4(reinterpret_cast<const struct sockaddr_in *>(nRes->ai_addr), false);
				break;
			}
			else if (AF_INET6 == res->ai_family)
			{
				h = new PSockAddrIpv6(reinterpret_cast<const struct sockaddr_in6 *>(nRes->ai_addr), false);
				break;
			}
		}
		freeaddrinfo(res);
		break;
	}
	default:
		break;
	}

	return h;
}

PSockAddrHandle PSockAddrCreateLocal(const char *ifIp, uint16_t port)
{
	PSockAddrBase *h = NULL;

	EPSocketInetType inetType = PSocketGetInetType(ifIp);
	switch (inetType)
	{
	case PS_INET_TYPE_IPV4:
		h = new PSockAddrIpv4(ifIp, port, true);
		break;
	case PS_INET_TYPE_IPV6:
		h = new PSockAddrIpv6(ifIp, port, true);
		break;
	default:
		break;
	}

	return h;
}

void PSockAddrDestroy(PSockAddrHandle *pHdl)
{
	if ((!pHdl) || (!(*pHdl)))
	{
		return;
	}

	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(*pHdl);
	delete h;
	*pHdl = NULL;
}

EPSocketInetType PSockAddrGetInetType(PSockAddrHandle hdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	return h->GetInetType();
}

const struct sockaddr *PSockAddrGetNative(PSockAddrHandle hdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	return h->GetNativeAddr();
}

const char *PSockAddrGetIp(PSockAddrHandle hdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	return h->GetIp().c_str();
}

uint16_t PSockAddrGetPort(PSockAddrHandle hdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	return h->GetPort();
}

const char *PSockAddrGetIfName(PSockAddrHandle hdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	return h->GetIfName().c_str();
}

int PSockAddrJoinMulticastGroup(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(groupAddr);
	if (!h->JoinGroup(fd, reinterpret_cast<PSockAddrBase *>(localAddr)))
	{
		return -1;
	}

	return 0;
}

int PSockAddrLeaveMulticastGroup(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(groupAddr);
	if (!h->LeaveGroup(fd, reinterpret_cast<PSockAddrBase *>(localAddr)))
	{
		return -1;
	}

	return 0;
}

int PSockAddrAddSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(groupAddr);
	if (!h->AddSource(fd, reinterpret_cast<PSockAddrBase *>(localAddr), reinterpret_cast<PSockAddrBase *>(sourceAddr)))
	{
		return -1;
	}

	return 0;
}

int PSockAddrDropSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(groupAddr);
	if (!h->DropSource(fd, reinterpret_cast<PSockAddrBase *>(localAddr), reinterpret_cast<PSockAddrBase *>(sourceAddr)))
	{
		return -1;
	}

	return 0;
}

int PSockAddrBlockSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(groupAddr);
	if (!h->BlockSource(fd, reinterpret_cast<PSockAddrBase *>(localAddr), reinterpret_cast<PSockAddrBase *>(sourceAddr)))
	{
		return -1;
	}

	return 0;
}

int PSockAddrUnblockSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(groupAddr);
	if (!h->UnblockSource(fd, reinterpret_cast<PSockAddrBase *>(localAddr), reinterpret_cast<PSockAddrBase *>(sourceAddr)))
	{
		return -1;
	}

	return 0;
}

int PSockAddrIncrease(PSockAddrHandle hdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	if (!h->Increase())
	{
		return -1;
	}

	return 0;
}

int PSockAddrDecrease(PSockAddrHandle hdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	if (!h->Decrease())
	{
		return -1;
	}

	return 0;
}

int PSockAddrCompare(PSockAddrHandle lhs, PSockAddrHandle rhs)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(lhs);
	if (!h)
	{
		return -2;
	}
	else
	{
		return h->Compare(reinterpret_cast<PSockAddrBase *>(rhs));
	}
}

int PSockAddrBind(int fd, PSockAddrHandle serverHdl, PSockAddrHandle localHdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(serverHdl);

	int on = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&on), sizeof(on));

#ifdef SO_REUSEPORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const char *>(&on), sizeof(on));
#endif

	if (!h->Bind(fd, reinterpret_cast<PSockAddrBase *>(localHdl)))
	{
		return -1;
	}

	return 0;
}

int PSockAddrAccept(int fd, PSockAddrHandle *pHdl)
{
	sockaddr_storage newAddr = { 0 };
	socklen_t newAddrLen = sizeof(sockaddr_storage);
	int newFd = static_cast<int>(accept(fd, reinterpret_cast<struct sockaddr *>(&newAddr), &newAddrLen));
	if (newFd < 0)
	{
		return -1;
	}

	PSockAddrBase *h = NULL;
	if (AF_INET == newAddr.ss_family)
	{
		h = new PSockAddrIpv4(reinterpret_cast<const struct sockaddr_in *>(&newAddr), false);
	}
	else if (AF_INET6 == newAddr.ss_family)
	{
		h = new PSockAddrIpv6(reinterpret_cast<const struct sockaddr_in6 *>(&newAddr), false);
	}
	else
	{
		return -1;
	}

	PSockAddrDestroy(pHdl);
	*pHdl = h;
	return newFd;
}

int PSockAddrConnect(int fd, PSockAddrHandle hdl)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	if (!h->Connect(fd))
	{
		return -1;
	}

	return 0;
}

int PSockAddrRecvfrom(int fd, PSockAddrHandle *pHdl, char *buf, int len)
{
	sockaddr_storage newAddr = { 0 };
	socklen_t newAddrLen = sizeof(sockaddr_storage);
	int recvLen = recvfrom(fd, buf, len, 0, reinterpret_cast<struct sockaddr *>(&newAddr), &newAddrLen);
	if (recvLen < 0)
	{
		return -1;
	}

	PSockAddrBase *h = NULL;
	if (AF_INET == newAddr.ss_family)
	{
		h = new PSockAddrIpv4(reinterpret_cast<const struct sockaddr_in *>(&newAddr), false);
	}
	else if (AF_INET6 == newAddr.ss_family)
	{
		h = new PSockAddrIpv6(reinterpret_cast<const struct sockaddr_in6 *>(&newAddr), false);
	}
	else
	{
		return -1;
	}

	PSockAddrDestroy(pHdl);
	*pHdl = h;
	return recvLen;
}

int PSockAddrSendto(int fd, PSockAddrHandle hdl, const char *buf, int len)
{
	PSockAddrBase *h = reinterpret_cast<PSockAddrBase *>(hdl);
	return h->Sendto(fd, buf, len);
}