#ifndef PLATFORM_PCAP_H_
#define PLATFORM_PCAP_H_

#include "stdint.h"
#include "PlatformBaseApi.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef void *PlatformPcapHandle;

	typedef void (*PPcapCallback)(void *privData, const uint8_t *data, size_t size);

	PLATFORM_BASE_API PlatformPcapHandle PlatformPcapUdpCreate(const char *ip, uint16_t port, const char *ifIp, int openPromisc);

	PLATFORM_BASE_API PlatformPcapHandle PlatformPcapFilterCreate(const char *sFilter, const char *ifIp, int openPromisc);

	PLATFORM_BASE_API void PlatformPcapDestroy(PlatformPcapHandle *pHdl);

	PLATFORM_BASE_API void PlatformPcapSetCallback(PlatformPcapHandle hdl, PPcapCallback cb, void *privData);

	PLATFORM_BASE_API int PlatformPcapStart(PlatformPcapHandle hdl);

	PLATFORM_BASE_API void PlatformPcapStop(PlatformPcapHandle hdl);

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_PCAP_H_