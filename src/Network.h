/*
	{{license}}
*/

#ifndef  __NETWORK_H__
#define  __NETWORK_H__
#ifdef _WIN32
#define  _WIN32_WINNT  0x0601
#define  ASIO_DISABLE_IOCP 1
#endif
#define  ASIO_STANDALONE 1
#include "asio.hpp"
#include "JSONPacket.h"
#include <functional>
#include <string>
class FireFoxDriver;



class INetworkDelegate
{
public:
	virtual asio::io_context& GetIOService() = 0;

	virtual void OnPacketRecevied(const JSONPacket&) = 0;

};


class Endpoint {

	INetworkDelegate& m_delagate;

	asio::ip::tcp::socket m_socket;

	string m_address;

	long   m_port;
	
	std::vector<char> m_buffer;

	std::string m_pendingpacket;

	JSONPacket m_lastpacket;

	string     m_messageToSend;

	void read_handler(const std::error_code&, std::size_t);

	bool read_message();

public :
	Endpoint(INetworkDelegate& delegator, const std::string&  address, long port);

	void Start();

	void Send(const JSONPacket& packet);


};


#endif  //__NETWORK_H__