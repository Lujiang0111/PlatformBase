#include <string.h>
#include "Socket/IPlatformSockAddrIpv4.h"

static int Ipv4Cmp(struct sockaddr_in *lhs, struct sockaddr_in *rhs)
{
	for (int index = 0; index < 4; index++)
	{
		if ((reinterpret_cast<uint8_t *>(&(lhs->sin_addr.s_addr)))[index] > (reinterpret_cast<uint8_t *>(&(rhs->sin_addr.s_addr)))[index])
		{
			return 1;
		}
		else if ((reinterpret_cast<uint8_t *>(&(lhs->sin_addr.s_addr)))[index] < (reinterpret_cast<uint8_t *>(&(rhs->sin_addr.s_addr)))[index])
		{
			return -1;
		}
	}

	return 0;
}

PSockAddrIpv4::PSockAddrIpv4(const char *ip, uint16_t port, bool isLocal)
{
	_ip = ip;
	_port = port;

	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = inet_addr(ip);
	_addr.sin_port = htons(port);

	if (isLocal)
	{
		SetLocalArgs();
	}
}

PSockAddrIpv4::PSockAddrIpv4(const struct sockaddr_in *pAddr, bool isLocal)
{
	memcpy(&_addr, pAddr, sizeof(struct sockaddr_in));

	char ip[INET_ADDRSTRLEN] = { 0 };
	inet_ntop(AF_INET, &_addr.sin_addr, ip, INET_ADDRSTRLEN);
	_ip = ip;
	_port = ntohs(_addr.sin_port);

	if (isLocal)
	{
		SetLocalArgs();
	}
}

void PSockAddrIpv4::SetLocalArgs()
{
#if defined (WIN32) || defined (_WINDLL)
#else
	struct ifaddrs *ifa = NULL;
	getifaddrs(&ifa);
	for (struct ifaddrs *nIfa = ifa; nIfa; nIfa = nIfa->ifa_next)
	{
		if ((NULL != nIfa->ifa_addr) && (AF_INET == nIfa->ifa_addr->sa_family))
		{
			if (0 == Ipv4Cmp(&_addr, reinterpret_cast<struct sockaddr_in *>(nIfa->ifa_addr)))
			{
				_ifName = nIfa->ifa_name;
				_ifNum = if_nametoindex(nIfa->ifa_name);
				break;
			}
		}
	}
	freeifaddrs(ifa);
#endif
}

PSockAddrIpv4::~PSockAddrIpv4()
{

}

bool PSockAddrIpv4::Increase()
{
	int index = 3;
	while (0xff == (reinterpret_cast<uint8_t *>(&(_addr.sin_addr.s_addr)))[index])
	{
		index--;
		if (index < 0)
		{
			return false;
		}
	}

	for (; index < 4; ++index)
	{
		++(reinterpret_cast<uint8_t *>(&(_addr.sin_addr.s_addr)))[index];
	}

	return true;
}

bool PSockAddrIpv4::Decrease()
{
	int index = 3;
	while (0 == (reinterpret_cast<uint8_t *>(&(_addr.sin_addr.s_addr)))[index])
	{
		index--;
		if (index < 0)
		{
			return false;
		}
	}

	for (; index < 4; ++index)
	{
		--(reinterpret_cast<uint8_t *>(&(_addr.sin_addr.s_addr)))[index];
	}

	return true;
}

int PSockAddrIpv4::Compare(PSockAddrBase *rhs)
{
	PSockAddrIpv4 *nRhs = dynamic_cast<PSockAddrIpv4 *>(rhs);
	if (!nRhs)
	{
		return -2;
	}

	return Ipv4Cmp(&_addr, &nRhs->_addr);
}

EPSocketInetType PSockAddrIpv4::GetInetType() const
{
	return PS_INET_TYPE_IPV4;
}

const struct sockaddr *PSockAddrIpv4::GetNativeAddr() const
{
	return reinterpret_cast<const struct sockaddr *>(&_addr);
}

bool PSockAddrIpv4::JoinGroup(int fd, PSockAddrBase *localAddr)
{
	PSockAddrIpv4 *nLocalAddr = dynamic_cast<PSockAddrIpv4 *>(localAddr);
	if (!nLocalAddr)
	{
		return false;
	}

	struct ip_mreq mreq = { 0 };
	mreq.imr_multiaddr.s_addr = _addr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = nLocalAddr->_addr.sin_addr.s_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(IP_ADD_MEMBERSHIP) fail! ip=%s, local ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv4::LeaveGroup(int fd, PSockAddrBase *localAddr)
{
	PSockAddrIpv4 *nLocalAddr = dynamic_cast<PSockAddrIpv4 *>(localAddr);
	if (!nLocalAddr)
	{
		return false;
	}

	struct ip_mreq mreq = { 0 };
	mreq.imr_multiaddr.s_addr = _addr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = nLocalAddr->_addr.sin_addr.s_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(IP_DROP_MEMBERSHIP) fail! ip=%s, local ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv4::AddSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr)
{
	PSockAddrIpv4 *nLocalAddr = dynamic_cast<PSockAddrIpv4 *>(localAddr);
	PSockAddrIpv4 *nSourceAddr = dynamic_cast<PSockAddrIpv4 *>(sourceAddr);
	if ((!nLocalAddr) || (!nSourceAddr))
	{
		return false;
	}

	struct ip_mreq_source mreq = { 0 };
	mreq.imr_multiaddr.s_addr = _addr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = nLocalAddr->_addr.sin_addr.s_addr;
	mreq.imr_sourceaddr.s_addr = nSourceAddr->_addr.sin_addr.s_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(IP_ADD_SOURCE_MEMBERSHIP) fail! ip=%s, local ip=%s, source ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), nSourceAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv4::DropSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr)
{
	PSockAddrIpv4 *nLocalAddr = dynamic_cast<PSockAddrIpv4 *>(localAddr);
	PSockAddrIpv4 *nSourceAddr = dynamic_cast<PSockAddrIpv4 *>(sourceAddr);
	if ((!nLocalAddr) || (!nSourceAddr))
	{
		return false;
	}

	struct ip_mreq_source mreq = { 0 };
	mreq.imr_multiaddr.s_addr = _addr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = nLocalAddr->_addr.sin_addr.s_addr;
	mreq.imr_sourceaddr.s_addr = nSourceAddr->_addr.sin_addr.s_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(IP_DROP_SOURCE_MEMBERSHIP) fail! ip=%s, local ip=%s, source ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), nSourceAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv4::BlockSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr)
{
	PSockAddrIpv4 *nLocalAddr = dynamic_cast<PSockAddrIpv4 *>(localAddr);
	PSockAddrIpv4 *nSourceAddr = dynamic_cast<PSockAddrIpv4 *>(sourceAddr);
	if ((!nLocalAddr) || (!nSourceAddr))
	{
		return false;
	}

	struct ip_mreq_source mreq = { 0 };
	mreq.imr_multiaddr.s_addr = _addr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = nLocalAddr->_addr.sin_addr.s_addr;
	mreq.imr_sourceaddr.s_addr = nSourceAddr->_addr.sin_addr.s_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_BLOCK_SOURCE, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(IP_BLOCK_SOURCE) fail! ip=%s, local ip=%s, source ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), nSourceAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv4::UnblockSource(int fd, PSockAddrBase *localAddr, PSockAddrBase *sourceAddr)
{
	PSockAddrIpv4 *nLocalAddr = dynamic_cast<PSockAddrIpv4 *>(localAddr);
	PSockAddrIpv4 *nSourceAddr = dynamic_cast<PSockAddrIpv4 *>(sourceAddr);
	if ((!nLocalAddr) || (!nSourceAddr))
	{
		return false;
	}

	struct ip_mreq_source mreq = { 0 };
	mreq.imr_multiaddr.s_addr = _addr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = nLocalAddr->_addr.sin_addr.s_addr;
	mreq.imr_sourceaddr.s_addr = nSourceAddr->_addr.sin_addr.s_addr;
	if (setsockopt(fd, IPPROTO_IP, IP_UNBLOCK_SOURCE, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "setsockopt(IP_UNBLOCK_SOURCE) fail! ip=%s, local ip=%s, source ip=%s, error=%d", _ip.c_str(), nLocalAddr->_ip.c_str(), nSourceAddr->_ip.c_str(), PSocketGetLastError());
		return false;
	}

	return true;
}

bool PSockAddrIpv4::Bind(int fd, PSockAddrBase *localAddr)
{
	PSockAddrIpv4 *nLocalAddr = dynamic_cast<PSockAddrIpv4 *>(localAddr);

#if defined (WIN32) || defined (_WINDLL)
	if (nLocalAddr)
	{
		if (bind(fd, reinterpret_cast<const struct sockaddr *>(&nLocalAddr->_addr), sizeof(struct sockaddr_in)) < 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "bind fail! ip=%s, port=%hu, error=%d", nLocalAddr->_ip.c_str(), nLocalAddr->_port, PSocketGetLastError());
			return false;
		}
	}
	else
	{
		if (bind(fd, reinterpret_cast<const struct sockaddr *>(&_addr), sizeof(struct sockaddr_in)) < 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "bind fail! ip=%s, port=%hu, error=%d", _ip.c_str(), _port, PSocketGetLastError());
			return false;
		}
	}
#else
	if (bind(fd, reinterpret_cast<const struct sockaddr *>(&_addr), sizeof(struct sockaddr_in)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "bind fail! ip=%s, port=%hu, error=%d", _ip.c_str(), _port, PSocketGetLastError());
		return false;
	}

	if (nLocalAddr)
	{

		if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, nLocalAddr->_ifName.c_str(), nLocalAddr->_ifName.length()) < 0)
		{
			PBLogOut(PL_LEVEL_ERROR, "setsockopt(SO_BINDTODEVICE) fail! local ip=%s, error=%d", nLocalAddr->_ip.c_str(), PSocketGetLastError());
			return false;
		}
	}
#endif

	return true;
}

bool PSockAddrIpv4::Connect(int fd)
{
	if (connect(fd, reinterpret_cast<const struct sockaddr *>(&_addr), sizeof(struct sockaddr_in)) < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "connect fail! ip=%s, port=%hu, error=%d", _ip.c_str(), _port, PSocketGetLastError());
		return false;
	}

	return true;
}

int PSockAddrIpv4::Sendto(int fd, const char *buf, int len)
{
	if (0 == len)
	{
		return 0;
	}

	return sendto(fd, buf, len, 0, reinterpret_cast<const struct sockaddr *>(&_addr), sizeof(struct sockaddr_in));
}