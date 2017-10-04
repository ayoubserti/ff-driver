/*
	{{license}}
*/

#ifndef  __NETWORK_H__
#define  __NETWORK_H__

#include <string>
#include <queue>

class FireFoxDriver;
class JSONPacket;
class Endpoint_Impl;

namespace std {
	class error_code;
}

namespace  asio{
	class io_context;
}

class INetworkDelegate
{
public:
	virtual asio::io_context& GetIOService() = 0;

	virtual void OnPacketRecevied(const JSONPacket&) = 0;

};


class Endpoint {

	Endpoint_Impl* m_impl;
	
protected:

	INetworkDelegate& m_delagate;

public :
	Endpoint(INetworkDelegate& delegator, Endpoint_Impl* impl);

	virtual void Start();

	virtual void Send(const JSONPacket& packet);


};


#endif  //__NETWORK_H__