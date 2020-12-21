#ifndef PLATFORM_SYNC_IO_H
#define PLATFORM_SYNC_IO_H

/****************************************/
/*				ͬ��IO�ӿ�				*/
/*										*/
/*   ע��������нӿ��ڲ�������������	*/
/*   ͬ����Ľӿھ���Ҫ�����̰߳�ȫ����	*/
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
	*�������ܣ�	����һ��ͬ��IO���
	*����������	timeoutMs	[IN]	��ʱʱ�䣬��λ����
	*����ֵ��	ͬ��IO���
	************************************************************************/
	PLATFORM_BASE_API PlatformSyncIoHandle PlatformSyncIoCreate(int timeoutMs);

	/************************************************************************
	*�������ܣ�	ɾ��һ��ͬ��IO���
	*����������	pHdl		[IN]	���ʵ��ָ��
	*ע�����	*pHdl����NULL
	************************************************************************/
	PLATFORM_BASE_API void PlatformSyncIoDestroy(PlatformSyncIoHandle *pHdl);

	/************************************************************************
	*�������ܣ�	����ʵ����IO������ʱʱ��
	*����������	hdl			[IN]	���ʵ��
	*			timeoutMs	[IN]	��ʱʱ�䣬��λ����
	************************************************************************/
	PLATFORM_BASE_API void PlatformSyncIoSetTimeout(PlatformSyncIoHandle hdl, int timeoutMs);

	/************************************************************************
	*�������ܣ�	ע��/ɾ���׽����¼�����
	*����������	hdl			[IN]	���ʵ��
	*			fd			[IN]	�׽���
	*			cb			[IN]	�¼���������ΪNULL��Ϊɾ���׽����¼�����
	*			timeoutMs	[IN]	��ʱʱ�䣬��λ����
	*			timeoutMs	[IN]	��ʱʱ�䣬��λ����
	************************************************************************/
	PLATFORM_BASE_API void PlatformSyncIoSetFdCallback(PlatformSyncIoHandle hdl, int fd, EventCallback cb, void *privData, int event);

	PLATFORM_BASE_API void PlatformSyncIoEnableEvent(PlatformSyncIoHandle hdl, int fd, int event);

	PLATFORM_BASE_API void PlatformSyncIoDisableEvent(PlatformSyncIoHandle hdl, int fd, int event);

	PLATFORM_BASE_API EPSIOResultCode PlatformSyncIoExecute(PlatformSyncIoHandle hdl);

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_SYNC_IO_H