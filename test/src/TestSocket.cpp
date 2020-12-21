#include <string.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include "PlatformSocket.h"
#include "TestSocket.h"

#if defined (WIN32) || defined (_WINDLL)
#include <WinSock2.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#include <sys/socket.h>
#define closesocket close
#define sleep_ms(x) usleep((x)*1000)
#endif

constexpr auto MAX_PKT_LEN = 65536;

extern bool bAppStart;

std::mutex workThreadMutex;

void TestTcpServer(int argc, char *argv[])
{
	std::string host;
	if (argc >= 3)
	{
		host = argv[2];
	}
	else
	{
		std::cout << "Input Host name(ip or domain):";
		std::cin >> host;
	}

	uint16_t port = 0;
	if (argc >= 4)
	{
		port = static_cast<uint16_t>(atoi(argv[3]));
	}
	else
	{
		std::cout << "Input port:";
		std::cin >> port;
	}

	PSockAddrHandle serverAddr = NULL, remoteAddr = NULL;
	int fd = -1, remoteFd = -1;
	do
	{
		serverAddr = PSockAddrCreate(host.c_str(), port);
		if (!serverAddr)
		{
			std::cerr << "cannot create server addr!\n";
			break;
		}

		EPSocketInetType inetType = PSocketGetInetType(PSockAddrGetIp(serverAddr));
		if (PS_INET_TYPE_IPV4 == inetType)
		{
			fd = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
		}
		else if (PS_INET_TYPE_IPV6 == inetType)
		{
			fd = static_cast<int>(socket(AF_INET6, SOCK_STREAM, 0));
		}

		if (fd < 0)
		{
			std::cerr << "worng inet type!\n";
			break;
		}

		// 设置socket缓冲区
		int opt = 1 << 24;	// 16MB
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&opt), sizeof(int));
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&opt), sizeof(int));

		if (PSockAddrBind(fd, serverAddr, NULL) < 0)
		{
			break;
		}

		if (listen(fd, 5) < 0)
		{
			break;
		}
		std::cout << "listening on " << PSockAddrGetIp(serverAddr) << ":" << PSockAddrGetPort(serverAddr) << "...\n";

		remoteFd = PSockAddrAccept(fd, &remoteAddr);
		if (remoteFd < 0)
		{
			break;
		}

		std::cout << "accept connect from " << PSockAddrGetIp(remoteAddr) << ":" << PSockAddrGetPort(remoteAddr) << std::endl;

		char recvBuf[MAX_PKT_LEN];
		while (bAppStart)
		{
			int recvLen = recv(remoteFd, recvBuf, MAX_PKT_LEN - 1, 0);
			recvBuf[recvLen] = 0;
			if (recvLen > 0)
			{
				std::cout << "recv from client: " << recvBuf << std::endl;

				// 退出标志
				if (0 == strcmp(recvBuf, "uquit"))
				{
					break;
				}

				send(remoteFd, recvBuf, recvLen, 0);
			}
			else
			{
				std::cout << "remote disconnect.\n";
				break;
			}
		}
	} while (0);

	PSockAddrDestroy(&serverAddr);
	PSockAddrDestroy(&remoteAddr);

	if (remoteFd > 0)
	{
		closesocket(remoteFd);
		remoteFd = -1;
	}

	if (fd > 0)
	{
		closesocket(fd);
		fd = -1;
	}
}

void TestTcpClient(int argc, char *argv[])
{
	std::string host;
	if (argc >= 3)
	{
		host = argv[2];
	}
	else
	{
		std::cout << "Input Host name(ip or domain):";
		std::cin >> host;
	}

	uint16_t port = 0;
	if (argc >= 4)
	{
		port = static_cast<uint16_t>(atoi(argv[3]));
	}
	else
	{
		std::cout << "Input port:";
		std::cin >> port;
	}

	PSockAddrHandle serverAddr = NULL;
	int fd = -1;
	do
	{
		serverAddr = PSockAddrCreate(host.c_str(), port);
		if (!serverAddr)
		{
			std::cerr << "cannot create server addr!\n";
			break;
		}

		EPSocketInetType inetType = PSocketGetInetType(PSockAddrGetIp(serverAddr));
		if (PS_INET_TYPE_IPV4 == inetType)
		{
			fd = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
		}
		else if (PS_INET_TYPE_IPV6 == inetType)
		{
			fd = static_cast<int>(socket(AF_INET6, SOCK_STREAM, 0));
		}

		if (fd < 0)
		{
			std::cerr << "worng inet type!\n";
			break;
		}

		// 设置socket缓冲区
		int opt = 1 << 24;	// 16MB
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&opt), sizeof(int));
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&opt), sizeof(int));

		std::cout << "connecting " << PSockAddrGetIp(serverAddr) << ":" << PSockAddrGetPort(serverAddr) << "...\n";
		if (PSockAddrConnect(fd, serverAddr) < 0)
		{
			break;
		}
		std::cout << "connected.\n";

		char recvBuf[MAX_PKT_LEN];
		while (bAppStart)
		{
			std::string sendMsg;
			std::cout << "Input Send Message:";
			std::cin >> sendMsg;

			// 退出标志
			if ("iquit" == sendMsg)
			{
				break;
			}

			int sendLen = send(fd, sendMsg.c_str(), static_cast<int>(sendMsg.length()), 0);
			if (sendLen > 0)
			{
				int recvLen = recv(fd, recvBuf, MAX_PKT_LEN - 1, 0);
				recvBuf[recvLen] = 0;
				if (recvLen > 0)
				{
					std::cout << "recv from Server: " << recvBuf << std::endl;
				}
				else
				{
					std::cout << "remote disconnect.\n";
					break;
				}
			}
			else
			{
				std::cout << "remote disconnect.\n";
				break;
			}
		}
	} while (0);

	PSockAddrDestroy(&serverAddr);

	if (fd > 0)
	{
		closesocket(fd);
		fd = -1;
	}
}

static void UdpRecvThread(int fd)
{
	std::cout << "recv thread start!\n";
	char recvBuf[MAX_PKT_LEN];
	while (bAppStart)
	{
		PSockAddrHandle remoteAddr = NULL;

		workThreadMutex.lock();
		int recvLen = PSockAddrRecvfrom(fd, &remoteAddr, recvBuf, MAX_PKT_LEN - 1);
		workThreadMutex.unlock();

		if (recvLen > 0)
		{
			recvBuf[recvLen] = 0;
			std::cout << "recv message form " << PSockAddrGetIp(remoteAddr) << ":" << PSockAddrGetPort(remoteAddr) << " : "<< recvBuf << std::endl;
		}
		else
		{
			sleep_ms(1);
		}

		PSockAddrDestroy(&remoteAddr);
	}
	std::cout << "recv thread stop!\n";
}

void TestUdpServer(int argc, char *argv[])
{
	std::string host;
	if (argc >= 3)
	{
		host = argv[2];
	}
	else
	{
		std::cout << "Input Host name(ip or domain):";
		std::cin >> host;
	}

	uint16_t port = 0;
	if (argc >= 4)
	{
		port = static_cast<uint16_t>(atoi(argv[3]));
	}
	else
	{
		std::cout << "Input port:";
		std::cin >> port;
	}

	std::string ifIp;
	if (PSocketIsMulticast(host.c_str()))
	{
		if (argc >= 5)
		{
			ifIp = argv[4];
		}
		else
		{
			std::cout << "Input interface ip:";
			std::cin >> ifIp;
		}
	}

	PSockAddrHandle serverAddr = NULL, localAddr = NULL;
	int fd = -1;
	do
	{
		serverAddr = PSockAddrCreate(host.c_str(), port);
		if (!serverAddr)
		{
			break;
		}

		EPSocketInetType inetType = PSocketGetInetType(PSockAddrGetIp(serverAddr));
		if (PS_INET_TYPE_IPV4 == inetType)
		{
			fd = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
		}
		else if (PS_INET_TYPE_IPV6 == inetType)
		{
			fd = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));
		}

		// 设置socket缓冲区
		int opt = 1 << 24;	// 16MB
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&opt), sizeof(int));
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&opt), sizeof(int));

		PSocketSetBlockMode(fd, 0);

		if (PSocketIsMulticast(host.c_str()))
		{
			localAddr = PSockAddrCreateLocal(ifIp.c_str(), port);
			if (!localAddr)
			{
				break;
			}

			if (PSockAddrBind(fd, serverAddr, localAddr) < 0)
			{
				break;
			}

			if (PSockAddrJoinMulticastGroup(fd, serverAddr, localAddr) < 0)
			{
				break;
			}
		}
		else
		{
			if (PSockAddrBind(fd, serverAddr, NULL) < 0)
			{
				break;
			}
		}

		std::thread *workThread = new std::thread(UdpRecvThread, fd);
		if (!workThread)
		{
			break;
		}

		if (PSocketIsMulticast(host.c_str()))
		{
			std::string cmd;
			std::string option;
			while (bAppStart)
			{
				std::cout << "input multicast command:";
				std::cin >> cmd >> option;
				std::lock_guard<std::mutex> lock(workThreadMutex);
				if ("j" == cmd)
				{
					PSockAddrHandle nAddr = PSockAddrCreate(option.c_str(), 0);
					if (nAddr)
					{
						PSockAddrJoinMulticastGroup(fd, nAddr, localAddr);
						PSockAddrDestroy(&nAddr);
					}
				}
				else if ("l" == cmd)
				{
					PSockAddrHandle nAddr = PSockAddrCreate(option.c_str(), 0);
					if (nAddr)
					{
						PSockAddrLeaveMulticastGroup(fd, nAddr, localAddr);
						PSockAddrDestroy(&nAddr);
					}
				}
				else if ("a" == cmd)
				{
					PSockAddrHandle nAddr = PSockAddrCreate(option.c_str(), 0);
					if (nAddr)
					{
						PSockAddrAddSource(fd, serverAddr, localAddr, nAddr);
						PSockAddrDestroy(&nAddr);
					}
				}
				else if ("d" == cmd)
				{
					PSockAddrHandle nAddr = PSockAddrCreate(option.c_str(), 0);
					if (nAddr)
					{
						PSockAddrDropSource(fd, serverAddr, localAddr, nAddr);
						PSockAddrDestroy(&nAddr);
					}
				}
				else if ("b" == cmd)
				{
					PSockAddrHandle nAddr = PSockAddrCreate(option.c_str(), 0);
					if (nAddr)
					{
						PSockAddrBlockSource(fd, serverAddr, localAddr, nAddr);
						PSockAddrDestroy(&nAddr);
					}
				}
				else if ("u" == cmd)
				{
					PSockAddrHandle nAddr = PSockAddrCreate(option.c_str(), 0);
					if (nAddr)
					{
						PSockAddrUnblockSource(fd, serverAddr, localAddr, nAddr);
						PSockAddrDestroy(&nAddr);
					}
				}
			}

			PSockAddrLeaveMulticastGroup(fd, serverAddr, localAddr);
		}

		workThread->join();
		delete workThread;
	} while (0);

	PSockAddrDestroy(&serverAddr);
	PSockAddrDestroy(&localAddr);

	if (fd > 0)
	{
		closesocket(fd);
		fd = -1;
	}
}

void TestUdpClient(int argc, char *argv[])
{
	std::string host;
	if (argc >= 3)
	{
		host = argv[2];
	}
	else
	{
		std::cout << "Input Host name(ip or domain):";
		std::cin >> host;
	}

	uint16_t port = 0;
	if (argc >= 4)
	{
		port = static_cast<uint16_t>(atoi(argv[3]));
	}
	else
	{
		std::cout << "Input port:";
		std::cin >> port;
	}

	std::string ifIp;
	if (PSocketIsMulticast(host.c_str()))
	{
		if (argc >= 5)
		{
			ifIp = argv[4];
		}
		else
		{
			std::cout << "Input interface ip:";
			std::cin >> ifIp;
		}
	}

	PSockAddrHandle serverAddr = NULL, localAddr = NULL;
	int fd = -1;
	do
	{
		serverAddr = PSockAddrCreate(host.c_str(), port);
		if (!serverAddr)
		{
			break;
		}

		EPSocketInetType inetType = PSocketGetInetType(PSockAddrGetIp(serverAddr));
		if (PS_INET_TYPE_IPV4 == inetType)
		{
			fd = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
		}
		else if (PS_INET_TYPE_IPV6 == inetType)
		{
			fd = static_cast<int>(socket(AF_INET6, SOCK_DGRAM, 0));
		}

		// 设置socket缓冲区
		int opt = 1 << 24;	// 16MB
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&opt), sizeof(int));
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&opt), sizeof(int));

		if (PSocketIsMulticast(host.c_str()))
		{
			localAddr = PSockAddrCreate(ifIp.c_str(), port);
			if (!localAddr)
			{
				break;
			}

			if (PSockAddrBind(fd, localAddr, NULL) < 0)
			{
				break;
			}
		}

		while (bAppStart)
		{
			std::string sendMsg;
			std::cout << "Input Send Message:";
			std::cin >> sendMsg;

			// 退出标志
			if ("iquit" == sendMsg)
			{
				break;
			}

			PSockAddrSendto(fd, serverAddr, sendMsg.c_str(), static_cast<int>(sendMsg.length()));
		}
	} while (0);

	PSockAddrDestroy(&serverAddr);
	PSockAddrDestroy(&localAddr);

	if (fd > 0)
	{
		closesocket(fd);
		fd = -1;
	}
}