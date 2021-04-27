#ifndef PLATFORM_SYNC_IP_SELECT_HPP_
#define PLATFORM_SYNC_IP_SELECT_HPP_

#include "PlatformSyncIo.h"

typedef struct Event_
{
	EventCallback cb;
	void *privData;
	int event;
}SEvent;

class PSIOSelect
{
public:
	PSIOSelect(int TimeoutMs = 0);
	virtual ~PSIOSelect();

	void SetTimeoutMs(int TimeoutMs);
	void SetFdCallback(int fd, EventCallback cb, void *privData, int event);
	void EnableEvent(int fd, int event);
	void DisableEvent(int fd, int event);
	EPSIOResultCode Execute();

private:
	PSIOSelect();
	PSIOSelect(const PSIOSelect &) = delete;
	PSIOSelect &operator=(const PSIOSelect &) = delete;

private:
	int _timeoutMs;
	std::unordered_map<int, SEvent *> _mapEvents;

#if defined(WIN32) || defined(_WINDLL)
	int _idleFd;	//windows不支持所有fd_set都空
#endif
};

#endif // !PLATFORM_SYNC_IP_SELECT_HPP_