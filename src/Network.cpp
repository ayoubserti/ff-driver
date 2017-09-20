#include "Network.h"
#include <string>

using namespace asio::ip;
using namespace std;


Endpoint::Endpoint( asio::io_service& parent, const std::string & address, long port)
	:m_socket(parent),
	m_currentmsg("")
{
	m_socket.connect(tcp::endpoint(address::from_string(address), port));
}

void Endpoint::Start()
{

}



void Endpoint::read_handler(const std::error_code  & e , std::size_t len)
{
	if (!e & len)
	{
		
	}
	

}