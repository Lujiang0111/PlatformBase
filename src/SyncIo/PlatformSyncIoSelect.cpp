#ifdef PLATFORM_SYNC_IO_SELECT

#if defined (WIN32) || defined (_WINDLL)
#include <WinSock2.h>
#else
#endif

#include <set>
#include <unordered_map>
#include "Base/IPlatformBaseApi.h"
#include "PlatformSocket.h"
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

PSIOSelect::PSIOSelect(int timeoutMs)
{
	_timeoutMs = timeoutMs;

	PSocketInitEnv();

#if defined(WIN32) || defined(_WINDLL)
	_idleFd = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
#endif
}

PSIOSelect::~PSIOSelect()
{
#if defined(WIN32) || defined(_WINDLL)
	if (_idleFd > 0)
	{
		closesocket(_idleFd);
	}
#endif

	PSocketCleanEnv();

	for (auto x = _mapEvents.begin(); x != _mapEvents.end(); ++x)
	{
		if (x->second)
		{
			delete x->second;
			x->second = NULL;
		}
	}
	_mapEvents.clear();
}

void PSIOSelect::SetTimeoutMs(int TimeoutMs)
{
	_timeoutMs = TimeoutMs;
}

void PSIOSelect::SetFdCallback(int fd, EventCallback cb, void *privData, int event)
{
	if (cb)
	{
		SEvent *pEvent = new SEvent();
		pEvent->cb = cb;
		pEvent->privData = privData;
		pEvent->event = event;

		auto x = _mapEvents.find(fd);
		if (x != _mapEvents.end())
		{
			if (x->second)
			{
				delete x->second;
				x->second = pEvent;
			}
		}
		else
		{
			_mapEvents[fd] = pEvent;
		}
	}
	else
	{
		auto x = _mapEvents.find(fd);
		if (x != _mapEvents.end())
		{
			if (x->second)
			{
				delete x->second;
				x->second = NULL;
			}
			_mapEvents.erase(x);
		}
	}
}

void PSIOSelect::EnableEvent(int fd, int event)
{
	auto x = _mapEvents.find(fd);
	if (x != _mapEvents.end())
	{
		if (x->second)
		{
			x->second->event |= event;
		}
	}
}

void PSIOSelect::DisableEvent(int fd, int event)
{
	auto x = _mapEvents.find(fd);
	if (x != _mapEvents.end())
	{
		if (x->second)
		{
			x->second->event &= ~event;
		}
	}
}

EPSIOResultCode PSIOSelect::Execute()
{
	fd_set rfds, wfds;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	timeval tv = { 0 };
	tv.tv_sec = _timeoutMs / 1000;
	tv.tv_usec = (_timeoutMs % 1000) * 1000;

	int maxFd = -1;
#if defined(WIN32) || defined(_WINDLL)
	maxFd = _idleFd;
	FD_SET(_idleFd, &rfds);
#endif

	for (auto x = _mapEvents.begin(); x != _mapEvents.end(); ++x)
	{
		if (x->first > maxFd)
		{
			maxFd = x->first;
		}

		if (x->second->event & PSIO_EVENT_READ)
		{
			FD_SET(x->first, &rfds);
		}
		else if (x->second->event & PSIO_EVENT_WRITE)
		{
			FD_SET(x->first, &wfds);
		}
	}

	EPSIOResultCode retCode = PSIO_RESULT_CODE_SUCCESS;
	int ret = select(maxFd + 1, &rfds, &wfds, NULL, &tv);
	if (ret < 0)
	{
		PBLogOut(PL_LEVEL_ERROR, "Select fail, error: %d", PSocketGetLastError());
		retCode = PSIO_RESULT_CODE_FAIL;
	}
	else if (ret == 0)
	{
		retCode = PSIO_RESULT_CODE_TIMEOUT;
	}
	else
	{
		std::set<int> setRemoveFd;
		for (auto x = _mapEvents.begin(); x != _mapEvents.end(); ++x)
		{
			// 先处理写
			if (FD_ISSET(x->first, &wfds))
			{
				if (PSIO_RESULT_CODE_FAIL == x->second->cb(x->second->privData, x->first, PSIO_EVENT_WRITE))
				{
					setRemoveFd.insert(x->first);
					continue;
				}
			}

			// 再处理读
			if (FD_ISSET(x->first, &rfds))
			{
				if (PSIO_RESULT_CODE_FAIL == x->second->cb(x->second->privData, x->first, PSIO_EVENT_READ))
				{
					setRemoveFd.insert(x->first);
					continue;
				}
			}
		}

		for (auto x = setRemoveFd.begin(); x != setRemoveFd.end(); ++x)
		{
			SetFdCallback(*x, NULL, NULL, NULL);
		}

		retCode = PSIO_RESULT_CODE_SUCCESS;
	}

	return retCode;
}

/******************************  C API START  ******************************/
PlatformSyncIoHandle PlatformSyncIoCreate(int timeoutMs)
{
	PSIOSelect *h = new PSIOSelect(timeoutMs);
	return h;
}

void PlatformSyncIoDestroy(PlatformSyncIoHandle *pHdl)
{
	if ((!pHdl) || (!(*pHdl)))
	{
		return;
	}

	PSIOSelect *h = (PSIOSelect *)(*pHdl);
	delete h;
	*pHdl = NULL;
}

void PlatformSyncIoSetTimeout(PlatformSyncIoHandle hdl, int timeoutMs)
{
	PSIOSelect *h = (PSIOSelect *)hdl;
	return h->SetTimeoutMs(timeoutMs);
}

void PlatformSyncIoSetFdCallback(PlatformSyncIoHandle hdl, int fd, EventCallback cb, void *privData, int event)
{
	PSIOSelect *h = (PSIOSelect *)hdl;
	return h->SetFdCallback(fd, cb, privData, event);
}

void PlatformSyncIoEnableEvent(PlatformSyncIoHandle hdl, int fd, int event)
{
	PSIOSelect *h = (PSIOSelect *)hdl;
	return h->EnableEvent(fd, event);
}

void PlatformSyncIoDisableEvent(PlatformSyncIoHandle hdl, int fd, int event)
{
	PSIOSelect *h = (PSIOSelect *)hdl;
	return h->DisableEvent(fd, event);
}

EPSIOResultCode PlatformSyncIoExecute(PlatformSyncIoHandle hdl)
{
	PSIOSelect *h = (PSIOSelect *)hdl;
	return h->Execute();
}
/*******************************  C API END  *******************************/

#endif