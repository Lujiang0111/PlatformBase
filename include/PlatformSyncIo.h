#ifndef PLATFORM_SYNC_IO_H
#define PLATFORM_SYNC_IO_H

/****************************************/
/*				同步IO接口				*/
/*										*/
/*   注意事项：所有接口内部均无锁，对相	*/
/*   同句柄的接口均需要考虑线程安全问题	*/
/****************************************/

#include "PlatformBaseApi.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PSIO_EVENT_READ 0x01
#define PSIO_EVENT_WRITE 0x02

	typedef enum PSIOResultCode_
	{
		PSIO_RESULT_CODE_SUCCESS = 0,
		PSIO_RESULT_CODE_FAIL,
		PSIO_RESULT_CODE_TIMEOUT,
	}EPSIOResultCode;

	typedef EPSIOResultCode(*EventCallback)(void *privData, int fd, int event);

	typedef void *PlatformSyncIoHandle;

	/************************************************************************
	*函数功能：	创建一个同步IO句柄
	*函数参数：	timeoutMs	[IN]	超时时间，单位毫秒
	*返回值：	同步IO句柄
	************************************************************************/
	PLATFORM_BASE_API PlatformSyncIoHandle PlatformSyncIoCreate(int timeoutMs);

	/************************************************************************
	*函数功能：	删除一个同步IO句柄
	*函数参数：	pHdl		[IN]	句柄实例指针
	*注意事项：	*pHdl会置NULL
	************************************************************************/
	PLATFORM_BASE_API void PlatformSyncIoDestroy(PlatformSyncIoHandle *pHdl);

	/************************************************************************
	*函数功能：	更新实例的IO操作超时时间
	*函数参数：	hdl			[IN]	句柄实例
	*			timeoutMs	[IN]	超时时间，单位毫秒
	************************************************************************/
	PLATFORM_BASE_API void PlatformSyncIoSetTimeout(PlatformSyncIoHandle hdl, int timeoutMs);

	/************************************************************************
	*函数功能：	注册/删除套接字事件处理
	*函数参数：	hdl			[IN]	句柄实例
	*			fd			[IN]	套接字
	*			cb			[IN]	事件处理函数，为NULL则为删除套接字事件处理
	*			timeoutMs	[IN]	超时时间，单位毫秒
	*			timeoutMs	[IN]	超时时间，单位毫秒
	************************************************************************/
	PLATFORM_BASE_API void PlatformSyncIoSetFdCallback(PlatformSyncIoHandle hdl, int fd, EventCallback cb, void *privData, int event);

	PLATFORM_BASE_API void PlatformSyncIoEnableEvent(PlatformSyncIoHandle hdl, int fd, int event);

	PLATFORM_BASE_API void PlatformSyncIoDisableEvent(PlatformSyncIoHandle hdl, int fd, int event);

	PLATFORM_BASE_API EPSIOResultCode PlatformSyncIoExecute(PlatformSyncIoHandle hdl);

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_SYNC_IO_H