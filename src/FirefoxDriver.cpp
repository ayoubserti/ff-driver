#include "FirefoxDriver.h"
#include "rapidjson/rapidjson.h"
#include <iostream>

constexpr int kMax_received = 10000; //10k


class JSONPacket {
	string m_msg;
public:
	JSONPacket(const string& msg) : m_msg(msg){}
	string Stringify() {
		string result;
		result += std::to_string(m_msg.size());
		result += ":";
		result += m_msg;
		return result;
	}

};

using namespace asio::ip;
FireFoxDriver::FireFoxDriver() :
	FirefoxProcess()
	,m_endpoint(m_ioservice)
{
	m_endpoint.connect(tcp::endpoint(address::from_string("127.0.0.1"), 6000));
	char buf[kMax_received];
	std::error_code error;
	std::size_t read = m_endpoint.read_some(asio::buffer(buf, kMax_received), error);
	buf[read] = '\0';
	std::cout << buf << std::endl;
}

std::vector<Tab> FireFoxDriver::GetTabList()
{
	std::vector<Tab> tabs;
	std::error_code error;
	JSONPacket jsonPacket("{ \"to\":\"root\", \"type\":\"listTabs\" }");
	string packetized = jsonPacket.Stringify();
	m_endpoint.write_some(asio::buffer(packetized,packetized.size()),error);

	if (error) {
		std::cout << "send failed: " << error.message() << std::endl;
	}
	else {
		char buf[kMax_received];
		std::size_t read = m_endpoint.read_some(asio::buffer(buf), error);
		if (error)
		{
			std::cout << "send failed: " << error.message() << std::endl;
		}
		buf[read] = '\0';
		std::cout << buf << std::endl;
	}

	return tabs;
}

Tab FireFoxDriver::OpenNewTab()
{
	return Tab();
}

void FireFoxDriver::NavigateTo(const Tab & inTab, std::string inUrl)
{
}

void FireFoxDriver::CloseTab(const Tab & inTab)
{
}

string Tab::GetURL()
{
	return m_TabURL;
}

string Tab::GetTitle()
{
	return m_title;
}

string Tab::GetActor()
{
	return m_tabActor;
}
