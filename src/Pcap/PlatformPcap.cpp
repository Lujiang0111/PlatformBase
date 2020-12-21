#if defined (WIN32) || defined (_WINDLL)
#else
#include <unistd.h>
#define closesocket close
#endif

#include "Socket/IPlatformSockAddrBase.h"
#include "Pcap/IPlatformPcap.h"

constexpr auto SIZE_ETHERNET = 14;
constexpr auto SIZE_LOOPBACK = 4;

// Ipv4 Header(rfc791, 3.1)
typedef struct Ipv4Header_
{
	uint8_t ver_ihl;				// Version (4 bits) + Internet header length (4 bits)
	uint8_t tos;					// Type of service
	uint16_t tlen;					// Total length
	uint16_t identification;		// Identification
	uint16_t flags_fo;				// Flags (3 bits) + Fragment offset (13 bits)
	uint8_t ttl;					// Time to live
	uint8_t proto;					// Protocol
	uint16_t crc;					// Header checksum
	struct in_addr ip_src;			// Source address
	struct in_addr ip_dst;			// Destination address
	uint32_t op_pad;				// Option + Padding
}SIpv4Header;

// Ipv6 Header(rfc2460, 3)
typedef struct Ipv6Header_
{
	union
	{
		struct ip6_hdrctl
		{
			uint32_t ip6_un1_flow;	// 4 bits version, 8 bits TC, 20 bits flow-ID
			uint16_t ip6_un1_plen;	// payload length
			uint8_t  ip6_un1_nxt;	// next header
			uint8_t  ip6_un1_hlim;	// hop limit
		} ip6_un1;
		uint8_t ip6_un2_vfc;		// 4 bits version, top 4 bits tclass
	} ip6_ctlun;
	struct in6_addr ip6_src;		// source address
	struct in6_addr ip6_dst;		// destination address
}SIpv6Header;

// Udp Header(rfc768)
typedef struct UdpHeader_
{
	uint16_t sport;					// Source port
	uint16_t dport;					// Destination port
	uint16_t len;					// Datagram length
	uint16_t crc;					// Checksum
}SUdpHeader;

#if (defined(WIN32)) || (defined(_WINDLL))
#include <tchar.h>
static bool PcapInitEnv()
{
	_TCHAR npcap_dir[512];
	UINT len = GetSystemDirectory(npcap_dir, 480);
	if (0 == len)
	{
		PBLogOut(PL_LEVEL_ERROR, "Error in GetSystemDirectory: %x", GetLastError());
		return false;
	}

	_tcscat_s(npcap_dir, 512, _T("\\Npcap"));
	if (0 == SetDllDirectory(npcap_dir))
	{
		PBLogOut(PL_LEVEL_ERROR, "Error in SetDllDirectory: %x", GetLastError());
		return FALSE;
	}
	return true;
}
#else
static bool PcapInitEnv()
{
	return true;
}
#endif

IPlatformPcap::IPlatformPcap(const char *ip, uint16_t port, const char *ifIp, bool openPromisc)
{
	_type = PP_TYPE_UDP;

	if (ip)
	{
		_ip = ip;
	}

	if (ifIp)
	{
		_ifIp = ifIp;
	}

	_port = port;
	_openPromisc = openPromisc;

	_pPcapCb = NULL;
	_pPcapCbPrivData = NULL;

	_fd = -1;
	_pcapHdl = NULL;
	_pcapType = 0;
	_capThread = NULL;
	_isRunning = false;
}

IPlatformPcap::IPlatformPcap(const char *sFilter, const char *ifIp, bool openPromisc)
{
	_type = PP_TYPE_FILTER;

	if (sFilter)
	{
		_sFilter = sFilter;
	}

	if (ifIp)
	{
		_ifIp = ifIp;
	}

	_openPromisc = openPromisc;

	_pPcapCb = NULL;
	_pPcapCbPrivData = NULL;

	_fd = -1;
	_pcapHdl = NULL;
	_pcapType = 0;
	_capThread = NULL;
	_isRunning = false;
}

IPlatformPcap::~IPlatformPcap()
{
	Stop();
}

void IPlatformPcap::SetCallback(PPcapCallback cb, void *privData)
{
	_pPcapCb = cb;
	_pPcapCbPrivData = privData;
}

bool IPlatformPcap::Start()
{
	PSocketInitEnv();

	if (!PcapInitEnv())
	{
		return false;
	}

	if (PP_TYPE_UDP == _type)
	{
		char sTmp[128];
		sprintf(sTmp, "dst host %s and dst port %hu", _ip.c_str(), _port);
		_sFilter = sTmp;

		// 如果是组播，则先要加入组播组
		if (PSocketIsMulticast(_ip.c_str()))
		{
			EPSocketInetType inetType = PSocketGetInetType(_ip.c_str());
			if (PS_INET_TYPE_IPV4 == inetType)
			{
				_fd = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
			}
			else if (PS_INET_TYPE_IPV6 == inetType)
			{
				_fd = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));
			}

			PSockAddrHandle serverAddr = PSockAddrCreate(_ip.c_str(), 0);
			PSockAddrHandle localAddr = PSockAddrCreateLocal(_ifIp.c_str(), 0);
			PSockAddrBind(_fd, serverAddr, localAddr);
			PSockAddrJoinMulticastGroup(_fd, serverAddr, localAddr);
			PSockAddrDestroy(&localAddr);
			PSockAddrDestroy(&serverAddr);
		}
	}

	pcap_if_t *alldevs = NULL;
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	if (-1 == pcap_findalldevs(&alldevs, errbuf))
	{
		PBLogOut(PL_LEVEL_ERROR, "Error in pcap_findalldevs: %s", errbuf);
		return false;
	}

	if (_ifIp.length() > 0)
	{
		char ip[INET6_ADDRSTRLEN] = { 0 };
		for (pcap_if_t *node = alldevs; node; node = node->next)
		{
			for (struct pcap_addr *addrNode = node->addresses; addrNode; addrNode = addrNode->next)
			{
				if (addrNode->addr)
				{
					if (AF_INET == addrNode->addr->sa_family)
					{
						struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in *>(addrNode->addr);
						inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
					}
					else if (AF_INET6 == addrNode->addr->sa_family)
					{
						struct sockaddr_in6 *addr = reinterpret_cast<struct sockaddr_in6 *>(addrNode->addr);
						inet_ntop(AF_INET6, &addr->sin6_addr, ip, INET6_ADDRSTRLEN);
					}
					else
					{
						continue;
					}

					if (0 == PSocketIpCompare(_ifIp.c_str(), ip))
					{
						_ifName = node->name;
						break;
					}
				}
			}

			if (("127.0.0.1" == _ifIp) && (PCAP_IF_LOOPBACK == (node->flags & PCAP_IF_LOOPBACK)))
			{
				_ifName = node->name;
			}

			if (_ifName.length() > 0)
			{
				break;
			}
		}
	}

	_isRunning = true;
	_capThread = new std::thread(CapThreadEntry, this);

	return true;
}

void IPlatformPcap::Stop()
{
	_isRunning = false;
	if (_capThread)
	{
		_capThread->join();
		_capThread = NULL;
	}

	if (_fd > 0)
	{
		closesocket(_fd);
		_fd = -1;
	}

	PSocketCleanEnv();
}

void IPlatformPcap::CapThreadEntry(void *privData)
{
	IPlatformPcap *h = reinterpret_cast<IPlatformPcap *>(privData);
	return h->CapThread();
}

void IPlatformPcap::CapThread()
{
	EPPcapStatus status = PP_STATUS_INIT;

	while (_isRunning)
	{
		switch (status)
		{
		case PP_STATUS_INIT:
			status = OpenCap();
			break;

		case PP_STATUS_WORK:
			status = DoCap();
			break;

		case PP_STATUS_ERROR:
			status = CloseCap();
			std::this_thread::sleep_for(std::chrono::seconds(1));
			break;

		default:
			status = PP_STATUS_ERROR;
			break;
		}
	}
	CloseCap();
}

EPPcapStatus IPlatformPcap::OpenCap()
{
	/* Open the adapter */
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	_pcapHdl = pcap_open_live((_ifName.length() > 0) ? _ifName.c_str() : NULL, 65536, _openPromisc ? 1 : 0, 1000, errbuf);
	if (!_pcapHdl)
	{
		PBLogOut(PL_LEVEL_ERROR, "Couldn't open device %s: %s", _ifName.c_str(), errbuf);
		return PP_STATUS_ERROR;
	}

	_pcapType = pcap_datalink(_pcapHdl);
	if ((DLT_NULL != _pcapType) && (DLT_EN10MB != _pcapType))
	{
		PBLogOut(PL_LEVEL_ERROR, "Device doesn't provide Ethernet headers - link type was %d\n", _pcapType);
	}

	struct bpf_program bfp = { 0 };
	u_int netmask = 0;

	/* Compile the filter */
	if (-1 == pcap_compile(_pcapHdl, &bfp, _sFilter.c_str(), 0, netmask))
	{
		PBLogOut(PL_LEVEL_ERROR, "Couldn't parse filter %s: %s", _sFilter.c_str(), pcap_geterr(_pcapHdl));
		return PP_STATUS_ERROR;
	}

	/* set the filter */
	if (-1 == pcap_setfilter(_pcapHdl, &bfp))
	{
		PBLogOut(PL_LEVEL_ERROR, "Couldn't install filter %s: %s", _sFilter.c_str(), pcap_geterr(_pcapHdl));
		return PP_STATUS_ERROR;
	}

	return PP_STATUS_WORK;
}

EPPcapStatus IPlatformPcap::DoCap()
{
	const u_char *packet;
	struct pcap_pkthdr *header;

	/* Grab a packet */
	int ret = pcap_next_ex(_pcapHdl, &header, &packet);
	if (ret < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "Get pcap error: %s!", pcap_geterr(_pcapHdl));
		return PP_STATUS_ERROR;
	}
	else if (ret == 0)
	{
		return PP_STATUS_WORK;
	}

	do
	{
		if (PP_TYPE_UDP == _type)
		{
			const uint8_t *ptr = packet;
			size_t size = header->len;

			if (DLT_NULL == _pcapType)
			{
				if (size <= SIZE_LOOPBACK)
				{
					break;
				}

				ptr += SIZE_LOOPBACK;
				size -= SIZE_LOOPBACK;
			}
			else if (DLT_EN10MB == _pcapType)
			{
				if (size <= SIZE_ETHERNET)
				{
					break;
				}

				ptr += SIZE_ETHERNET;
				size -= SIZE_ETHERNET;
			}

			int ipVersion = ((*ptr) >> 4) & 0x0f;

			if (4 == ipVersion)			// Ipv4
			{
				uint8_t ipv4Size = ((*ptr) & 0x0f) * 4;
				if (size <= ipv4Size)
				{
					break;
				}
				const SIpv4Header *ipv4Header = reinterpret_cast<const SIpv4Header *>(ptr);
				ptr += ipv4Size;
				size -= ipv4Size;

				if (17 == ipv4Header->proto)	// UDP Protocol
				{
					if (size <= sizeof(SUdpHeader))
					{
						break;
					}
					const SUdpHeader *udpHeader = reinterpret_cast<const SUdpHeader *>(ptr);
					ptr += sizeof(SUdpHeader);
					size -= sizeof(SUdpHeader);

					if (_pPcapCb)
					{
						_pPcapCb(_pPcapCbPrivData, ptr, ntohs(udpHeader->len) - sizeof(SUdpHeader));
					}
				}
			}
			else if (6 == ipVersion)	// Ipv6
			{

			}
		}
		else
		{
			if (_pPcapCb)
			{
				_pPcapCb(_pPcapCbPrivData, packet, header->len);
			}
		}
	} while (0);

	return PP_STATUS_WORK;
}

EPPcapStatus IPlatformPcap::CloseCap()
{
	if (_pcapHdl)
	{
		pcap_close(_pcapHdl);
		_pcapHdl = NULL;
	}

	return PP_STATUS_INIT;
}

/******************************  C API START  ******************************/
PlatformPcapHandle PlatformPcapUdpCreate(const char *ip, uint16_t port, const char *ifIp, int openPromisc)
{
	IPlatformPcap *h = NULL;
	h = new IPlatformPcap(ip, port, ifIp, (openPromisc != 0));
	return h;
}

PlatformPcapHandle PlatformPcapFilterCreate(const char *sFilter, const char *ifIp, int openPromisc)
{
	IPlatformPcap *h = NULL;
	h = new IPlatformPcap(sFilter, ifIp, (openPromisc != 0));
	return h;
}

void PlatformPcapDestroy(PlatformPcapHandle *pHdl)
{
	if ((!pHdl) || (!(*pHdl)))
	{
		return;
	}

	IPlatformPcap *h = reinterpret_cast<IPlatformPcap *>(*pHdl);
	delete h;
	*pHdl = NULL;
}

void PlatformPcapSetCallback(PlatformPcapHandle hdl, PPcapCallback cb, void *privData)
{
	IPlatformPcap *h = reinterpret_cast<IPlatformPcap *>(hdl);
	return h->SetCallback(cb, privData);
}

int PlatformPcapStart(PlatformPcapHandle hdl)
{
	IPlatformPcap *h = reinterpret_cast<IPlatformPcap *>(hdl);
	if (!h->Start())
	{
		return -1;
	}

	return 0;
}

void PlatformPcapStop(PlatformPcapHandle hdl)
{
	IPlatformPcap *h = reinterpret_cast<IPlatformPcap *>(hdl);
	h->Stop();
}
/*******************************  C API END  *******************************/