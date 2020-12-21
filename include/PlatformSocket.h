#ifndef PLATFORM_SOCKET_H_
#define PLATFORM_SOCKET_H_

#if defined (WIN32) || defined (_WINDLL)
#else
#include <netinet/in.h>
#endif

#include "PlatformBaseApi.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum PSocketInetType_
	{
		PS_INET_TYPE_UNKNOWN = 0,
		PS_INET_TYPE_IPV4,
		PS_INET_TYPE_IPV6,
	}EPSocketInetType;

	/************************************************************************
	*�������ܣ�	��ʼ���׽��ֻ���,win�µ���WSAStartupʹ��v2.2��,linux��ʵ���޲���.ʹ�ñ�ģ��
	*			ʱ��Ҫ�ȵ��ô˺���
	*����ֵ��	0�ɹ�,����ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSocketInitEnv();

	/************************************************************************
	*�������ܣ�	�����׽��ֻ���,win�µ���WSACleanup,linux��ʵ���޲���
	*����ֵ��	0�ɹ�,����ʧ��
	************************************************************************/
	PLATFORM_BASE_API void PSocketCleanEnv();

	/************************************************************************
	*�������ܣ�	��ȡ���һ��ʧ�ܵ���Ϣ
	*����ֵ��	���һ��ʧ�ܵ���Ϣ
	************************************************************************/
	PLATFORM_BASE_API int PSocketGetLastError();

	/************************************************************************
	*�������ܣ�	�����װ��ֵ�����ģʽ
	*����������	fd			[IN]	��Ҫ������װ���
	*			mode		[IN]	0������ģʽ,1����ģʽ
	*����ֵ��	0�ɹ�,����ʧ��
	*ע�����	�׽���Ĭ��Ϊ����ģʽ
	************************************************************************/
	PLATFORM_BASE_API int PSocketSetBlockMode(int fd, int mode);

	/************************************************************************
	*�������ܣ�	TCP�׽��ֵ�keepalive����
	*����������	fd			[IN]	��Ҫ������װ���
	*			timeoutMs	[IN]	keepalive�ĳ�ʱʱ��(��λ����),0����ر�,
	*								��0ʱ��Сʱ��Ϊ2S,����2S��2S����
	*����ֵ��	0�ɹ�,����ʧ��
	*ע�����	Ĭ���׽����ǲ�����keepalive��
	************************************************************************/
	PLATFORM_BASE_API int PSocketSetTcpKeepalive(int fd, int timeoutMs);

	/************************************************************************
	*�������ܣ�	UDP�׽��ֵ�ttl����
	*����������	fd			[IN]	��Ҫ������װ���
	*			ttl			[IN]	ttlֵ
	*����ֵ��	0�ɹ�,����ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSocketSetUdpTtl(int fd, int ttl);

	/************************************************************************
	*�������ܣ�	��ѯip��ַ��ip����
	*����������	ip			[IN]	ip��ַ
	*����ֵ��	ip����
	************************************************************************/
	PLATFORM_BASE_API EPSocketInetType PSocketGetInetType(const char *ip);

	/************************************************************************
	*�������ܣ�	��ѯip��ַ�Ƿ�Ϊ�鲥��ַ
	*����������	ip			[IN]	ip��ַ
	*����ֵ��	0	���鲥��ַ
				-1	�����鲥��ַ
	************************************************************************/
	PLATFORM_BASE_API int PSocketIsMulticast(const char *ip);

	/************************************************************************
	*�������ܣ�	ip��ַ�Ƚ�
	*�������:	lhs			[IN]	ip��ַ
				rhs			[IN]	ip��ַ
	*����ֵ��	-1	lhs < rhs
				0	lhs = rhs
				1	lhs > rhs
				-2	�޷��Ƚ�
	************************************************************************/
	PLATFORM_BASE_API int PSocketIpCompare(const char *lhs, const char *rhs);

	/************************************************************************/
	/* ���½ӿڶ�sockaddr���м򵥷�װ����ʵ�ֶ�IPV4��IPV6��ͬʱ֧��			    */
	/************************************************************************/
	typedef void *PSockAddrHandle;

	/************************************************************************
	*�������ܣ�	ͨ����������PSockAddrHandleʵ��
	*�������:	host		[IN]	ip��ַ��������
				port		[IN]	�˿�
	*����ֵ��	ʵ�����
	*ע�����	����ʹ��ʱ����PSockAddrDestroy����ʵ��
	************************************************************************/
	PLATFORM_BASE_API PSockAddrHandle PSockAddrCreate(const char *host, uint16_t port);

	/************************************************************************
	*�������ܣ�	ͨ��IP��ַ��������PSockAddrHandleʵ��
	*�������:	ifIp		[IN]	����ip��ַ
				port		[IN]	�˿�
	*����ֵ��	ʵ�����
	*ע�����	�������ص�ַʵ��ʱ����ô˽ӿڣ��ӿڻ��Զ���ȡ������Ϣ
				����ʹ��ʱ����PSockAddrDestroy����ʵ��
	************************************************************************/
	PLATFORM_BASE_API PSockAddrHandle PSockAddrCreateLocal(const char *ifIp, uint16_t port);

	/************************************************************************
	*�������ܣ�	����PSockAddrHandleʵ��
	*�������:	pHdl		[IN]	ʵ�����ָ��
	*ע�����	*pHdl����NULL
	************************************************************************/
	PLATFORM_BASE_API void PSockAddrDestroy(PSockAddrHandle *pHdl);

	/************************************************************************
	*�������ܣ�	����PSockAddrHandleʵ����ԭ��struct sockaddr *�ṹ
	*�������:	hdl			[IN]	ʵ�����
	*����ֵ��	ԭ��struct sockaddr *�ṹ
	***********************************************************************/
	PLATFORM_BASE_API const struct sockaddr *PSockAddrGetNativeAddr(PSockAddrHandle hdl);

	/************************************************************************
	*�������ܣ�	����PSockAddrHandleʵ����ip��ַ
	*�������:	hdl			[IN]	ʵ�����
	*����ֵ��	ip��ַ
	************************************************************************/
	PLATFORM_BASE_API const char *PSockAddrGetIp(PSockAddrHandle hdl);

	/************************************************************************
	*�������ܣ�	����PSockAddrHandleʵ���Ķ˿�
	*�������:	hdl			[IN]	ʵ�����
	*����ֵ��	�˿�
	************************************************************************/
	PLATFORM_BASE_API uint16_t PSockAddrGetPort(PSockAddrHandle hdl);

	/************************************************************************
	*�������ܣ�	���ر���PSockAddrHandleʵ������������
	*�������:	hdl			[IN]	ʵ�����
	*����ֵ��	��������
	************************************************************************/
	PLATFORM_BASE_API const char *PSockAddrGetIfName(PSockAddrHandle hdl);

	/************************************************************************
	*�������ܣ�	socket�����鲥��
	*�������:	hdl			[IN]	ʵ�����
				groupAddr	[IN]	�鲥��ַ
				localAddr	[IN]	������ַ
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrJoinMulticastGroup(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr);

	/************************************************************************
	*�������ܣ�	socket�뿪�鲥��
	*�������:	hdl			[IN]	ʵ�����
				groupAddr	[IN]	�鲥��ַ
				localAddr	[IN]	������ַ
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrLeaveMulticastGroup(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr);

	/************************************************************************
	*�������ܣ�	�鲥�����Դ
	*�������:	hdl			[IN]	ʵ�����
				groupAddr	[IN]	�鲥��ַ
				localAddr	[IN]	������ַ
				sourceAddr	[IN]	Դ��ַ
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrAddSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr);

	/************************************************************************
	*�������ܣ�	�鲥��ɾ��Դ
	*�������:	hdl			[IN]	ʵ�����
				groupAddr	[IN]	�鲥��ַ
				localAddr	[IN]	������ַ
				sourceAddr	[IN]	Դ��ַ
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrDropSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr);

	/************************************************************************
	*�������ܣ�	�鲥������Դ
	*�������:	hdl			[IN]	ʵ�����
				groupAddr	[IN]	�鲥��ַ
				localAddr	[IN]	������ַ
				sourceAddr	[IN]	Դ��ַ
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrBlockSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr);

	/************************************************************************
	*�������ܣ�	�鲥��������Դ
	*�������:	hdl			[IN]	ʵ�����
				groupAddr	[IN]	�鲥��ַ
				localAddr	[IN]	������ַ
				sourceAddr	[IN]	Դ��ַ
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrUnblockSource(int fd, PSockAddrHandle groupAddr, PSockAddrHandle localAddr, PSockAddrHandle sourceAddr);

	/************************************************************************
	*�������ܣ�	ip��ַ+1���˿ڲ���
	*�������:	hdl			[IN]	ʵ�����
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrIncrease(PSockAddrHandle hdl);

	/************************************************************************
	*�������ܣ�	ip��ַ-1���˿ڲ���
	*�������:	hdl			[IN]	ʵ�����
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrDecrease(PSockAddrHandle hdl);

	/************************************************************************
	*�������ܣ�	ip��ַ�Ƚ�
	*�������:	lhs			[IN]	��ʵ�����
				rhs			[IN]	��ʵ�����
	*����ֵ��	-1	lhs < rhs
				0	lhs = rhs
				1	lhs > rhs
				-2	�޷��Ƚ�
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrCompare(PSockAddrHandle lhs, PSockAddrHandle rhs);

	/************************************************************************
	*�������ܣ�	socket bind��װ
	*�������:	fd			[IN]	socket
				serverHdl	[IN]	Ŀ���ַ���
				localHdl	[IN]	���ص�ַ��������ڽ����鲥ʱָ������������ΪNULL
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	*ע�����	�ڲ����Զ�����reuse addr��reuse port
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrBind(int fd, PSockAddrHandle serverHdl, PSockAddrHandle localHdl);

	/************************************************************************
	*�������ܣ�	socket accept��װ
	*�������:	fd			[IN]	socket
				pHdl		[OUT]	accept��ȡ��ʵ�������ַ
	*����ֵ��	>0	�ɹ���ֵΪaccept��ȡ��fd
				-1	ʧ��
	*ע�����	��(*hdl)ԭ�������ݣ�������ݻᱻ�滻
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrAccept(int fd, PSockAddrHandle *pHdl);

	/************************************************************************
	*�������ܣ�	socket connect��װ
	*�������:	fd			[IN]	socket
				hdl			[IN]	ʵ�����
	*����ֵ��	0	�ɹ�
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrConnect(int fd, PSockAddrHandle hdl);

	/************************************************************************
	*�������ܣ�	socket recvfrom��װ
	*�������:	fd			[IN]	socket
				pHdl		[OUT]	recvform��ȡ��ʵ�������ַ
				buf			[INOUT]	д���ַ
				len			[IN]	���д�볤��
	*����ֵ��	>0	�ɹ���ֵΪ�����ֽڳ���
				=0	��ʾ�Զ��ѹر�����
				-1	ʧ��
	*ע�����	��(*hdl)ԭ�������ݣ�������ݻᱻ�滻
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrRecvfrom(int fd, PSockAddrHandle *pHdl, char *buf, int len);

	/************************************************************************
	*�������ܣ�	socket sendto��װ
	*�������:	fd			[IN]	socket
				hdl			[IN]	ʵ�����
				buf			[IN]	���͵�ַ
				len			[IN]	���ͳ���
	*����ֵ��	>=0	�ɹ���ֵΪ�����ֽڳ���
				-1	ʧ��
	************************************************************************/
	PLATFORM_BASE_API int PSockAddrSendto(int fd, PSockAddrHandle hdl, const char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif // !PLATFORM_SOCKET_H_