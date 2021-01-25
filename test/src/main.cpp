#include <signal.h>
#include <iostream>
#include "PlatformSocket.h"
#include "TestFile.h"
#include "TestLog.h"
#include "TestSocket.h"
#include "TestPcap.h"
#include "TestSyncIo.h"

bool bAppStart = true;

static void SigIntHandler(int sig_num)
{
	signal(SIGINT, SigIntHandler);
	bAppStart = false;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, SigIntHandler);

	PSocketInitEnv();

	int testNo = 0;
	if (argc >= 2)
	{
		testNo = atoi(argv[1]);
	}
	else
	{
		std::cout << "Test List\n" <<
			"1: Test listing File\n" <<
			"2: Test Fast Logging\n" <<
			"3: Test Tcp Server\n" <<
			"4: Test Tcp Client\n" <<
			"5: Test Udp Server\n" <<
			"6: Test Udp Client\n" <<
			"7: Test Pcap IP Port\n" <<
			"8: Test Pcap Filter\n" <<
			"Input TestNo:";
		std::cin >> testNo;
	}

	switch (testNo)
	{
	case 1:
		TestFileSort(argc, argv);
		break;
	case 2:
		TestLogLimit(argc, argv);
		break;
	case 3:
		TestTcpServer(argc, argv);
		break;
	case 4:
		TestTcpClient(argc, argv);
		break;
	case 5:
		TestUdpServer(argc, argv);
		break;
	case 6:
		TestUdpClient(argc, argv);
		break;
#if ENABLE_PCAP
	case 7:
		TestPcapIpPort(argc, argv);
		break;
	case 8:
		TestPcapFilter(argc, argv);
		break;
#endif
	default:
		std::cout << "out of range, byebye!\n";
		break;
	}

	PSocketCleanEnv();

	return 0;
}