#include <string.h>
#include "Socket/IPlatformSockAddrIpv6.h"

static int Ipv6Cmp(struct sockaddr_in6 *lhs, struct sockaddr_in6 *rhs)
{
	for (int index = 0; index < 16; index++)
	{
		if ((reinterpret_cast<uint8_t *>(&(lhs->sin6_addr)))[index] > (reinterpret_cast<uint8_t *>(&(rhs->sin6_addr)))[index])
		{
			return 1;
		}
		else if ((reinterpret_cast<uint8_t *>(&(lhs->sin6_addr)))[index] < (reinterpret_cast<uint8_t *>(&(rhs->sin6_addr)))[index])
		{
			return -1;
		}
	}
	return 0;
}

PSockAddrIpv6::PSockAddrIpv6(const char *ip, uint16_t port, bool isLocal)
{
	_ip = ip;
	_port = port;
	_scopeId = 0;

	memset(&_addr, 0, sizeof(_addr));
	_addr.sin6_family = AF_INET6;
	inet_pton(AF_INET6, ip, _addr.sin6_addr.s6_addr);
	_addr.sin6_port = htons(port);

	if (isLocal)
	{
		SetLocalArgs();
	}
}

PSockAddrIpv6::PSockAddrIpv6(const struct sockaddr_in6 *pAddr, bool isLocal)
{
	memcpy(&_addr, pAddr, sizeof(struct sockaddr_in6));

	char ip[INET6_ADDRSTRLEN] = { 0 };
	inet_ntop(AF_INET6, &_addr.sin6_addr, ip, INET6_ADDRSTRLEN);
	_ip = ip;
	_port = ntohs(_addr.sin6_port);
	_scopeId = 0;

	if (isLocal)
	{
		SetLocalArgs();
	}
}

void PSockAddrIpv6::SetLocalArgs()
{
#if defined (WIN32) || defined (_WINDLL)
#else
	struct ifaddrs *ifa = NULL;
	getifaddrs(&ifa);
	for (struct ifaddrs *nIfa = ifa; nIfa; nIfa = nIfa->ifa_next)
	{
		if ((NULL != nIfa->ifa_addr) && (AF_INET6 == nIfa->ifa_addr->sa_family))
		{
			if (0 == Ipv6Cmp(&_addr, reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_addr)))
			{
				_ifName = nIfa->ifa_name;
				_ifNum = if_nametoindex(nIfa->ifa_name);
				_scopeId = (reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_addr))->sin6_scope_id;
				break;
			}
		}
	}
	freeifaddrs(ifa);
#endif
}

PSockAddrIpv6::~PSockAddrIpv6()
{

}

bool PSockAddrIpv6::Increase()
{
	int index = 15;
	while (0xff == (reinterpret_cast<uint8_t *>(&(_addr.sin6_addr)))[index])
	{
		index--;
		if (index < 0)
		{
			return false;
		}
	}

	for (; index < 16; ++index)
	{
		++(reinterpret_cast<uint8_t *>(&(_addr.sin6_addr)))[index];
	}

	return true;
}

bool PSockAddrIpv6::Decrease()
{
	int index = 15;
	while (0 == (reinterpret_cast<uint8_t *>(&(_addr.sin6_addr)))[index])
	{
		index--;
		if (index < 0)
		{
			return false;
		}
	}

	for (; index < 16; ++index)
	{
		--(reinterpret_cast<uint8_t *>(&(_addr.sin6_addr)))[index];
	}

	return true;
}

int PSockAddrIpv6::Compare(PSockAddrBase *rhs)
{
	PSockAddrIpv6 *nRhs = dynamic_cast<PSockAddrIpv6 *>(rhs);
	if (!nRhs)
	{
		return -2;
	}

	return Ipv6Cmp(&_addr, &nRhs->_addr);
}

EPSocketInetType PSockAddrIpv6::GetInetType() const
{
	return PS_INET_TYPE_IPV6;
}

const struct sockaddr *PSockAddrIpv6::GetNativeAddr() const
{
	return reinterpret_cast<const struct sockaddr *>(&_addr);
}

bool PSockAddrIpv6::JoinGroup(int fd, PSockAddrBase *localAddr)
{
	PSockAddrIpv6 *nLocalAddr = dynamic_cast<PSockAddrIpv6 *>(localAddr);
	if (!nLocalAddr)
	{
		return false;
	}

	struct ipv6_mreq mreq6;
	memcpy(&mreq6.ipv6mr_multiaddr, &(_addr.sin6_addr), sizeof(struct in6_addr));
	mreq6.ipv6mr_interface = nLocalAddr->_ifNum;
	if (setsockopt(fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(IPV6_ADD_MEMBERSHIP) fail! ip=%s, local ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv6::LeaveGroup(int fd, PSockAddrBase *localAddr)
{
	PSockAddrIpv6 *nLocalAddr = dynamic_cast<PSockAddrIpv6 *>(localAddr);
	if (!nLocalAddr)
	{
		return false;
	}

	struct ipv6_mreq mreq6 = { 0 };
	memcpy(&mreq6.ipv6mr_multiaddr, &(_addr.sin6_addr), sizeof(struct in6_addr));
	mreq6.ipv6mr_interface = nLocalAddr->_ifNum;
	if (setsockopt(fd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(IPV6_DROP_MEMBERSHIP) fail! ip=%s, local ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv6::AddSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr)
{
	PSockAddrIpv6 *nLocalAddr = dynamic_cast<PSockAddrIpv6 *>(localAddr);
	PSockAddrIpv6 *nSourceAddr = dynamic_cast<PSockAddrIpv6 *>(sourceAddr);
	if ((!nLocalAddr) || (!nSourceAddr))
	{
		return false;
	}

	struct group_source_req mreq6 = { 0 };
	memcpy(&mreq6.gsr_group, &(_addr.sin6_addr), sizeof(struct in6_addr));
	memcpy(&mreq6.gsr_source, &(nSourceAddr->_addr.sin6_addr), sizeof(struct in6_addr));
	mreq6.gsr_interface = nLocalAddr->_ifNum;
	if (setsockopt(fd, IPPROTO_IPV6, MCAST_JOIN_SOURCE_GROUP, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(MCAST_JOIN_SOURCE_GROUP) fail! ip=%s, local ip=%s, source ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), nSourceAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv6::DropSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr)
{
	PSockAddrIpv6 *nLocalAddr = dynamic_cast<PSockAddrIpv6 *>(localAddr);
	PSockAddrIpv6 *nSourceAddr = dynamic_cast<PSockAddrIpv6 *>(sourceAddr);
	if ((!nLocalAddr) || (!nSourceAddr))
	{
		return false;
	}

	struct group_source_req mreq6 = { 0 };
	memcpy(&mreq6.gsr_group, &(_addr.sin6_addr), sizeof(struct in6_addr));
	memcpy(&mreq6.gsr_source, &(nSourceAddr->_addr.sin6_addr), sizeof(struct in6_addr));
	mreq6.gsr_interface = nLocalAddr->_ifNum;
	if (setsockopt(fd, IPPROTO_IPV6, MCAST_LEAVE_SOURCE_GROUP, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(MCAST_LEAVE_SOURCE_GROUP) fail! ip=%s, local ip=%s, source ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), nSourceAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv6::BlockSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr)
{
	PSockAddrIpv6 *nLocalAddr = dynamic_cast<PSockAddrIpv6 *>(localAddr);
	PSockAddrIpv6 *nSourceAddr = dynamic_cast<PSockAddrIpv6 *>(sourceAddr);
	if ((!nLocalAddr) || (!nSourceAddr))
	{
		return false;
	}

	struct group_source_req mreq6 = { 0 };
	memcpy(&mreq6.gsr_group, &(_addr.sin6_addr), sizeof(struct in6_addr));
	memcpy(&mreq6.gsr_source, &(nSourceAddr->_addr.sin6_addr), sizeof(struct in6_addr));
	mreq6.gsr_interface = nLocalAddr->_ifNum;
	if (setsockopt(fd, IPPROTO_IPV6, MCAST_BLOCK_SOURCE, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(MCAST_BLOCK_SOURCE) fail! ip=%s, local ip=%s, source ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), nSourceAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv6::UnblockSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr)
{
	PSockAddrIpv6 *nLocalAddr = dynamic_cast<PSockAddrIpv6 *>(localAddr);
	PSockAddrIpv6 *nSourceAddr = dynamic_cast<PSockAddrIpv6 *>(sourceAddr);
	if ((!nLocalAddr) || (!nSourceAddr))
	{
		return false;
	}

	struct group_source_req mreq6 = { 0 };
	memcpy(&mreq6.gsr_group, &(_addr.sin6_addr), sizeof(struct in6_addr));
	memcpy(&mreq6.gsr_source, &(nSourceAddr->_addr.sin6_addr), sizeof(struct in6_addr));
	mreq6.gsr_interface = nLocalAddr->_ifNum;
	if (setsockopt(fd, IPPROTO_IPV6, MCAST_UNBLOCK_SOURCE, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(MCAST_UNBLOCK_SOURCE) fail! ip=%s, local ip=%s, source ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), nSourceAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv6::Bind(int fd, PSockAddrBase *localAddr)
{
	PSockAddrIpv6 *nLocalAddr = dynamic_cast<PSockAddrIpv6 *>(localAddr);

#if defined (WIN32) || defined (_WINDLL)
	if (nLocalAddr)
	{
		if (bind(fd, reinterpret_cast<const struct sockaddr *>(&nLocalAddr->_addr), sizeof(struct sockaddr_in6)) < 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "bind fail! ip=%s, port=%hu, error=%d", nLocalAddr->_ip.c_str(), nLocalAddr->_port, PSocketGetLastError());
			return false;
		}
	}
	else
	{
		if (bind(fd, reinterpret_cast<const struct sockaddr *>(&_addr), sizeof(struct sockaddr_in6)) < 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "bind fail! ip=%s, port=%hu, error=%d", _ip.c_str(), _port, PSocketGetLastError());
			return false;
		}
	}
#else
	if (bind(fd, reinterpret_cast<const struct sockaddr *>(&_addr), sizeof(struct sockaddr_in6)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "bind fail! ip=%s, port=%hu, error=%d", _ip.c_str(), _port, PSocketGetLastError());
		return false;
	}

	if (nLocalAddr)
	{
		_scopeId = nLocalAddr->_scopeId;
	}
#endif

	return true;
}

bool PSockAddrIpv6::Connect(int fd)
{
	if (connect(fd, reinterpret_cast<const struct sockaddr *>(&_addr), sizeof(struct sockaddr_in6)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "connect fail! ip=%s, port=%hu, error=%d", _ip.c_str(), _port, PSocketGetLastError());
		return false;
	}

	return true;
}

int PSockAddrIpv6::Sendto(int fd, const char *buf, int len)
{
	if (0 == len)
	{
		return 0;
	}

	return sendto(fd, buf, len, 0, reinterpret_cast<const struct sockaddr *>(&_addr), sizeof(struct sockaddr_in6));
}