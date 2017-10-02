#ifndef __NETWORK_IMPL_H__
#define __NETWORK_IMPL_H__

#ifdef _WIN32
#define  _WIN32_WINNT  0x0601
#define  ASIO_DISABLE_IOCP 1
#endif
#define  ASIO_STANDALONE 1
#include "asio.hpp"
#include "JSONPacket.h"
#include "JSONPacket_Impl.h"
#include "Network.h"

using namespace asio::ip;
using namespace std;


class Endpoint_Impl : public Endpoint
{
	asio::ip::tcp::socket m_socket;

	string m_address;

	long   m_port;

	std::vector<char> m_buffer;

	std::string m_pendingpacket;

	JSONPacket_Impl m_lastpacket;

	queue<string>     m_messagesToSend;
	string			m_messageToSend;

public:

	Endpoint_Impl(INetworkDelegate& delegator, const std::string&  address, long port)
		: Endpoint(delegator, this),
		m_socket(delegator.GetIOService()),
		m_buffer(1024),
		m_pendingpacket(""),
		m_lastpacket("")
	{
		m_address = address;
		m_port = port;
	}

	void Start()
	{
		m_socket.async_connect(tcp::endpoint(address::from_string(m_address), (short)m_port), [&](const std::error_code& err) {

			m_socket.async_read_some(asio::buffer(m_buffer), std::bind(&Endpoint_Impl::read_handler, this, std::placeholders::_1, std::placeholders::_2));
		});

	}

	void Send(const JSONPacket & packet)
	{

		m_messagesToSend.push(packet.Stringify());
		m_messageToSend = m_messagesToSend.front();

		m_socket.async_write_some(asio::buffer(m_messageToSend), std::bind(&Endpoint_Impl::send_handler, this, placeholders::_1, placeholders::_2));
	}
	void read_handler(const std::error_code  & e, std::size_t len)
	{
		if (!e && len)
		{
			m_pendingpacket.append(m_buffer.begin(), m_buffer.begin() + len);
			while (read_message()) {}
		}
		m_socket.async_read_some(asio::buffer(m_buffer), std::bind(&Endpoint_Impl::read_handler, this, std::placeholders::_1, std::placeholders::_2));
	}

	bool read_message()
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

		m_lastpacket = JSONPacket_Impl(m_pendingpacket.substr(0, packetlen));
		m_delagate.OnPacketRecevied(m_lastpacket);

		m_pendingpacket = m_pendingpacket.substr(packetlen);

		return true;
	}

	void send_handler(const std::error_code &ec, std::size_t)
	{
		if (!ec)
		{

			if (m_messagesToSend.size() > 0)
			{
				m_messagesToSend.pop();
				if (!m_messagesToSend.empty())
				{
					m_messageToSend = m_messagesToSend.front();
					m_socket.async_write_some(asio::buffer(m_messageToSend), std::bind(&Endpoint_Impl::send_handler, this, placeholders::_1, placeholders::_2));
				}
			}

		}
	}
};




#endif