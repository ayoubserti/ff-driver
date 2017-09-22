#include "Network.h"
#include <string>

using namespace asio::ip;
using namespace std;


Endpoint::Endpoint(INetworkDelegate& delegator, const std::string & address, long port)
	:m_delagate(delegator),
	m_socket(delegator.GetIOService()),
	m_buffer(1024),
	m_pendingpacket(""),
	m_lastpacket("")
{
	m_address = address;
	m_port = port;
}

void Endpoint::Start()
{
	m_socket.async_connect(tcp::endpoint(address::from_string(m_address), m_port), [&](const std::error_code& err) {
			
		m_socket.async_read_some(asio::buffer(m_buffer), std::bind(&Endpoint::read_handler, this, std::placeholders::_1, std::placeholders::_2));
	});
	
}

void Endpoint::Send(const JSONPacket & packet)
{
	JSONPacket reply("");
	JSONPacket request(packet);

	m_messageToSend = request.Stringify();

	m_socket.async_write_some(asio::buffer(m_messageToSend), [&](const std::error_code  & e, std::size_t len) {

			
	});
}



void Endpoint::read_handler(const std::error_code  & e , std::size_t len)
{
	if (!e && len)
	{
		m_pendingpacket.append(m_buffer.begin(),m_buffer.begin() +len);
		while (read_message()) {}

		
	}
	m_socket.async_read_some(asio::buffer(m_buffer), std::bind(&Endpoint::read_handler, this, std::placeholders::_1, std::placeholders::_2));
}

bool Endpoint::read_message()
{
	if (m_pendingpacket == "") return false;
	auto pos = m_pendingpacket.find(":");
	if (pos<0)
		return false;
	size_t packetlen = std::stol(m_pendingpacket.substr(0, pos + 1));
	if (m_pendingpacket.size() - (pos + 1) < packetlen)
	{
		return false;
	}

	m_pendingpacket = m_pendingpacket.substr(pos + 1);
	
	m_lastpacket = JSONPacket(m_pendingpacket.substr(0, packetlen));

	m_delagate.OnPacketRecevied(m_lastpacket);

	m_pendingpacket = m_pendingpacket.substr(packetlen);
	
	return true;
}
