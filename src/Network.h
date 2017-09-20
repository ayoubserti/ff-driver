/*
	{{license}}
*/

#ifndef  __NETWORK_H__
#define  __NETWORK_H__

#include "asio.hpp"
#include "JSONPacket.h"

class FireFoxDriver;
namespace std {
	class string;
	class error_code;
}

class Endpoint {

	asio::ip::tcp::socket m_socket;

	JSONPacket m_currentmsg;

	char buffer_[2048];

	void read_handler(const std::error_code&, std::size_t);

public :
	Endpoint( asio::io_service& parent, const std::string&  address, long port);

	void Start();


};


#endif  //__NETWORK_H__