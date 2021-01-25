#if ENABLE_PCAP

#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include "PlatformPcap.h"
#include "TestPcap.h"

extern bool bAppStart;

typedef struct PcapIPPort_
{
	std::string ip;
	uint16_t port;
	std::string ifIp;
	int openPromisc;
	FILE *fp;
}SPcapIPPort;

typedef struct PcapFilter_
{
	std::string filter;
	std::string ifIp;
	int openPromisc;
	FILE *fp;
}SPcapFilter;

static void TestPcapIpPortCb(void *privData, const uint8_t *data, size_t size)
{
	SPcapIPPort *h = reinterpret_cast<SPcapIPPort *>(privData);
	if (h->fp)
	{
		fwrite(data, size, 1, h->fp);
		fflush(h->fp);
	}
	std::cout << "cap " << size << " bytes\n";
}

void TestPcapIpPort(int argc, char *argv[])
{
	SPcapIPPort *h = new SPcapIPPort();

	if (argc >= 3)
	{
		h->ip = argv[2];
	}
	else
	{
		std::cout << "Input ip:";
		std::cin >> h->ip;
	}

	if (argc >= 4)
	{
		h->port = static_cast<uint16_t>(atoi(argv[3]));
	}
	else
	{
		std::cout << "Input port:";
		std::cin >> h->port;
	}

	if (argc >= 5)
	{
		h->ifIp = argv[4];
	}
	else
	{
		std::cout << "Input interface ip:";
		std::cin >> h->ifIp;
	}

	if (argc >= 6)
	{
		h->openPromisc = atoi(argv[5]);
	}
	else
	{
		std::cout << "Input openPromisc:";
		std::cin >> h->openPromisc;
	}

	PlatformPcapHandle pcapHdl = PlatformPcapUdpCreate(h->ip.c_str(), h->port, h->ifIp.c_str(), h->openPromisc);
	PlatformPcapSetCallback(pcapHdl, TestPcapIpPortCb, h);
	PlatformPcapStart(pcapHdl);

	char fileName[2048] = { 0 };
	sprintf(fileName, "%s_%hu_%s.rec", h->ip.c_str(), h->port, h->ifIp.c_str());
	h->fp = fopen(fileName, "wb");

	while (bAppStart)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	if (h->fp)
	{
		fclose(h->fp);
		h->fp = NULL;
	}

	PlatformPcapDestroy(&pcapHdl);
	delete h;
}

static void TestPcapFilterCb(void *privData, const uint8_t *data, size_t size)
{
	SPcapFilter *h = reinterpret_cast<SPcapFilter *>(privData);
	if (h->fp)
	{
		fwrite(data, size, 1, h->fp);
		fflush(h->fp);
	}
	std::cout << "cap " << size << " bytes\n";
}

void TestPcapFilter(int argc, char *argv[])
{
	SPcapFilter *h = new SPcapFilter();

	if (argc >= 3)
	{
		h->filter = argv[2];
	}
	else
	{
		std::cout << "Input filter:";
		std::cin >> h->filter;
	}

	if ("NULL" == h->filter)
	{
		h->filter = "";
	}

	if (argc >= 4)
	{
		h->ifIp = argv[3];
	}
	else
	{
		std::cout << "Input interface ip:";
		std::cin >> h->ifIp;
	}

	if (argc >= 5)
	{
		h->openPromisc = atoi(argv[4]);
	}
	else
	{
		std::cout << "Input openPromisc:";
		std::cin >> h->openPromisc;
	}

	PlatformPcapHandle pcapHdl = PlatformPcapFilterCreate(h->filter.c_str(), h->ifIp.c_str(), h->openPromisc);
	PlatformPcapSetCallback(pcapHdl, TestPcapFilterCb, h);
	PlatformPcapStart(pcapHdl);

	char fileName[2048] = { 0 };
	sprintf(fileName, "%s_%s.rec", h->filter.c_str(), h->ifIp.c_str());
	h->fp = fopen(fileName, "wb");
	if (h->fp)
	{
		fclose(h->fp);
		h->fp = fopen(fileName, "ab");
	}

	while (bAppStart)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	if (h->fp)
	{
		fclose(h->fp);
		h->fp = NULL;
	}

	PlatformPcapDestroy(&pcapHdl);
	delete h;
}

#endif