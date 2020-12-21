#include "Socket/IPlatformSockAddrBase.h"

PSockAddrBase::PSockAddrBase()
{
	_port = 0;
	_ifNum = 0;
}

PSockAddrBase::~PSockAddrBase()
{

}

const std::string &PSockAddrBase::GetIp() const
{
	return _ip;
}

uint16_t PSockAddrBase::GetPort() const
{
	return _port;
}

const std::string &PSockAddrBase::GetIfName() const
{
	return _ifName;
}