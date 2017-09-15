#define RAPIDJSON_HAS_STDSTRING 1
#include "FirefoxDriver.h"
#undef GetObject
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <iostream>


constexpr int kMax_received = 10000; //10k
constexpr size_t  kMax_len_len = 20; // max  length variable width



using namespace asio::ip;


JSONPacket FireFoxDriver::_ReadOneJSONPacket()
{
	JSONPacket p("");
	string value;
	char buf[kMax_len_len] = { 0 };
	std::error_code error;
	size_t len = m_endpoint.read_some(asio::buffer(buf,kMax_len_len-1), error);
	if (error)
	{
		//throw exception 
	}
	else
	{
		buf[len] = '\0';
		string tmp = buf;
		
		size_t offset = tmp.find(':');
		size_t packetlen = std::stol(tmp.substr(0, offset + 1));
		std::vector<char> reply(packetlen);
		m_endpoint.read_some(asio::buffer(reply));
		tmp.append(reply.begin(), reply.end());

		return JSONPacket::Parse(tmp);

	}
	return p;
}

JSONPacket FireFoxDriver::_SendRequest(const string & msg)
{
	JSONPacket reply("");
	JSONPacket request(msg);
	
	string requestStr = request.Stringify();

	
	std::error_code error;

	m_endpoint.write_some(asio::buffer(requestStr), error);
	if (!error)
	{
		reply = _ReadOneJSONPacket();
	}

	return reply;

}

FireFoxDriver::FireFoxDriver() :
	FirefoxProcess()
	,m_endpoint(m_ioservice)
{
	m_endpoint.connect(tcp::endpoint(address::from_string("127.0.0.1"), 6000));

	//when connected; server send some usefull information about  connection
	
	JSONPacket p = _ReadOneJSONPacket();
	

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
		JSONPacket reply = _ReadOneJSONPacket();
		if (error)
		{
			std::cerr << "send failed: " << error.message() << std::endl;
		}
		else
		{
			
			rapidjson::Document document;
			if (document.Parse(reply.GetMsg()).HasParseError())
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
					tab.m_consoleActor = tabObj["consoleActor"].GetString();
					tabs.push_back(tab);
				}

			}

		}
		

	}

	return tabs;
}

Tab FireFoxDriver::OpenNewTab()
{
	Tab tab;
	auto tabs = GetTabList();
	if (tabs.size())
	{
		//doesn't harm to check;) 
		
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("to", tabs[0].GetConsoleActor(), doc.GetAllocator());
		doc.AddMember("type", "evaluateJS", doc.GetAllocator());
		doc.AddMember("text", "window.open(\"about:blank\")",doc.GetAllocator());

		//serialiaze msg
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		
		JSONPacket reply = _SendRequest(buffer.GetString());
		_ReadOneJSONPacket(); // read notification packet
	
		tab.m_TabURL = "";

		
	}
	return tab;
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

	//read reply to free endpoint buffer 
	char buf[kMax_received];
	
	std::error_code error;
	size_t len = m_endpoint.read_some(asio::buffer(buf), error);
	

}

void FireFoxDriver::CloseTab(const Tab & inTab)
{
	//as there is no Request msg to close a Tab; we evaluate 'window.close()'in the Tab
	//this hack works only with tab open using a script
	rapidjson::Document msg(rapidjson::kObjectType);

	msg.SetObject();
	msg.AddMember("to", inTab.GetConsoleActor(), msg.GetAllocator());

	msg.AddMember("type", "evaluateJS", msg.GetAllocator());

	msg.AddMember("text", "window.close()", msg.GetAllocator());
	

	//serialiaze msg
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	msg.Accept(writer);

	JSONPacket json(buffer.GetString());
	m_endpoint.write_some(asio::buffer(json.Stringify()));

	//read reply to free endpoint buffer 
	char buf[kMax_received];

	std::error_code error;
	size_t len = m_endpoint.read_some(asio::buffer(buf), error);

	if (!error)
	{

	}

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

string Tab::GetConsoleActor() const
{
	return m_consoleActor;
}
