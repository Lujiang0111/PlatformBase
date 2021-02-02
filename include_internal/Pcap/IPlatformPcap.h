#ifndef I_PLATFORM_PCAP_H_
#define I_PLATFORM_PCAP_H_

#include <mutex>
#include <thread>
#include "pcap.h"
#include "PlatformPcap.h"

typedef enum PPcapType_
{
	PP_TYPE_UDP = 0,
	PP_TYPE_FILTER,
	PP_TYPE_DUMP,
}EPPcapType;

typedef enum PPcapStatus_
{
	PP_STATUS_INIT = 0,
	PP_STATUS_WORK,
	PP_STATUS_ERROR,
}EPPcapStatus;

class IPlatformPcap
{
public:
	IPlatformPcap(const char *ip, uint16_t port, const char *ifIp, bool openPromisc);
	IPlatformPcap(const char *sFilter, const char *ifIp, bool openPromisc);
	IPlatformPcap(const char *sFilter, const char *ifIp, bool openPromisc, const char *dumpFileName);
	virtual ~IPlatformPcap();

	void UpdateCallback(PPcapCallback cb, void *privData);
	void UpdateDumpFileName(const char *dumpFileName);

	bool Start();
	void Stop();

private:
	IPlatformPcap() = delete;
	IPlatformPcap(const IPlatformPcap&) = delete;
	IPlatformPcap& operator=(const IPlatformPcap&) = delete;

	void Init();

	static void CapThreadEntry(void *privData);
	void CapThread();

	EPPcapStatus OpenCap();
	EPPcapStatus DoCap();
	EPPcapStatus CloseCap();
	void UpdateCap();

private:
	EPPcapType _type;
	std::string _ip;
	std::string _ifIp;
	std::string _sFilter;
	std::string _ifName;
	uint16_t _port;
	bool _openPromisc;

	PPcapCallback _pPcapCb;
	void *_pPcapCbPrivData;
	PPcapCallback _newPPcapCb;
	void *_newPPcapCbPrivData;
	bool _isCallbackUpdate;

	int _fd;
	pcap_t *_pcapHdl;
	int _pcapType;

	std::thread *_capThread;
	std::mutex _capThreadMutex;
	bool _isRunning;

	pcap_dumper_t *_pcapDumpHdl;
	std::string _dumpFileName;
	bool _isDumpFileNameUpdate;
};

#endif // !I_PLATFORM_PCAP_H_