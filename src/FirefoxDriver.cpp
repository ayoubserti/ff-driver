#define RAPIDJSON_HAS_STDSTRING 1
#include "FirefoxDriver.h"
#undef GetObject
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

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

	static JSONPacket Parse(const string& jsonmsg) {

		JSONPacket packet("");
		size_t offset = jsonmsg.find(":");
		if (offset>0)
		{
			packet.m_msg =  jsonmsg.substr(offset+1);

		}

		return packet;
			
	}

	string GetMsg()
	{
		return m_msg;
	}

};

using namespace asio::ip;


FireFoxDriver::FireFoxDriver() :
	FirefoxProcess()
	,m_endpoint(m_ioservice)
{
	m_endpoint.connect(tcp::endpoint(address::from_string("127.0.0.1"), 6000));

	//when connected; server send some usefull information about  connection
	//TODO parse and store it


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
			std::cerr << "send failed: " << error.message() << std::endl;
		}
		buf[read] = '\0';
		
		rapidjson::Document document;
		if (document.Parse(JSONPacket::Parse(buf).GetMsg()).HasParseError())
		{
			std::cerr << "Error while Parsing recieved JSON:  " << document.GetParseError() << std::endl;
		}
		else
		{
			auto obj = document.GetObject();
			const auto& tabsArray = obj["tabs"].GetArray();
			for (auto& it : tabsArray)
			{
				auto tabObj = it.GetObject();
				Tab tab;
				tab.m_title = tabObj["title"].GetString();
				tab.m_tabActor = tabObj["actor"].GetString();
				tab.m_TabURL = tabObj["url"].GetString();
				tabs.push_back(tab);
			}

		}
		

	}

	return tabs;
}

Tab FireFoxDriver::OpenNewTab()
{
	//delayed;
	return Tab();
}

void FireFoxDriver::NavigateTo(const Tab & inTab, std::string inUrl)
{
	
	rapidjson::Document msg(rapidjson::kObjectType);

	msg.SetObject();
	msg.AddMember("to", inTab.GetActor(), msg.GetAllocator());

	msg.AddMember("type", "navigateTo", msg.GetAllocator());
	msg.AddMember("url", inUrl, msg.GetAllocator());
	
	//serialiaze msg
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	msg.Accept(writer);

	JSONPacket json(buffer.GetString());
	m_endpoint.write_some(asio::buffer(json.Stringify()));
	
}

void FireFoxDriver::CloseTab(const Tab & inTab)
{
}

string Tab::GetURL() const
{
	return m_TabURL;
}

string Tab::GetTitle() const
{
	return m_title;
}

string Tab::GetActor() const
{
	return m_tabActor;
}
