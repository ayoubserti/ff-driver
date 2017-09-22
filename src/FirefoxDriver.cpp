#define RAPIDJSON_HAS_STDSTRING 1
#include "FirefoxDriver.h"
#if _WIN32
#undef GetObject
#endif
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <iostream>
#include <regex>

constexpr size_t  kMax_len_len = 20; // max  packet 'length' representation

using namespace asio::ip;


vector<string> FireFoxDriver::s_unsolicitedEvents = {"tabListChanged"};

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
		size_t packetlen = std::stol(tmp.substr(0, offset + 1)), trailling = packetlen - len +offset +1;

		std::vector<char> reply(trailling);
		
		asio::read(m_endpoint, asio::buffer(reply), asio::transfer_exactly(trailling));
		tmp.append(reply.begin(), reply.end());
		p = JSONPacket::Parse(tmp);
		rapidjson::Document doc;
		if (!doc.Parse(p.GetMsg()).HasParseError())
		{
			if (doc.HasMember("type") &&  std::find(s_unsolicitedEvents.begin(), s_unsolicitedEvents.end(),doc["type"].GetString()) != s_unsolicitedEvents.end())
				return _ReadOneJSONPacket();
		}
		
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
#if ASYNC_API
	,m_asyncEndpoint(*this,"127.0.0.1",6000)
#endif
{
#if !ASYNC_API
	m_endpoint.connect(tcp::endpoint(address::from_string("127.0.0.1"), 6000));


	//when connected; server send some usefull information about  connection
	
	JSONPacket p = _ReadOneJSONPacket();
#else
	
	m_asyncEndpoint.Start();
#endif

	m_status = eWaitingHandShake;


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
				//check actor
				string actor = obj["from"].GetString();
				if (actor == "root" && obj.HasMember("tabs"))
				{
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
		

	}

	return tabs;
}

void FireFoxDriver::GetTabList(function<void(const vector<Tab>&)>&& inCB)
{
	m_activeRequests["root"] = [=](const JSONPacket& packet) {

		rapidjson::Document document;
		if (document.Parse(packet.GetMsg()).HasParseError())
		{
			std::cerr << "Error while Parsing recieved JSON:  " << document.GetParseError() << std::endl;
			return;
		}
		auto obj = document.GetObject();
		//check actor
		string actor = obj["from"].GetString();
		if (actor == "root" && obj.HasMember("tabs"))
		{
			vector<Tab> tabs;
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
			inCB(tabs);
		} 
	};
	
	m_ioservice.dispatch([&]() {
		JSONPacket jsonPacket("{ \"to\":\"root\", \"type\":\"listTabs\" }");
		m_asyncEndpoint.Send(jsonPacket);
	});
	
}

Tab FireFoxDriver::OpenNewTab(const string& url)
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
		string scriptText = "window.open('";
		scriptText += url;
		scriptText += "')";
		doc.AddMember("text", scriptText,doc.GetAllocator());

		//serialiaze msg
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		
		JSONPacket reply = _SendRequest(buffer.GetString());
		
	
		tab.m_TabURL = "";

		
	}
	return tab;
}

void FireFoxDriver::NavigateTo(const Tab & inTab, const std::string& inUrl)
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

	_ReadOneJSONPacket();

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

	_ReadOneJSONPacket();

}

void FireFoxDriver::ReloadTab(const Tab & inTab)
{
	string msg = "{\"to\":\":actor\", \"type\": \"reload\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab.GetActor());
	_SendRequest(msg);
}


const string FireFoxDriver::EvaluateJS(const Tab& inTab, const string& inScript)
{
	rapidjson::Document doc;
	doc.SetObject();
	doc.AddMember("to", inTab.GetConsoleActor(), doc.GetAllocator());
	doc.AddMember("type", "evaluateJS", doc.GetAllocator());
	doc.AddMember("text", inScript, doc.GetAllocator());

	JSONPacket reply("");
	//serialiaze msg
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		reply = _SendRequest(buffer.GetString());

	}
	
	//parse result
	doc.Parse(reply.GetMsg());
	string result = "";
	rapidjson::Value valOfdoc = doc.GetObject();

	
#if 0
	//code deactivated for lather.  
	/*
		
	*/

	if (valOfdoc["result"].IsObject())
	{
		//object 
		auto res = valOfdoc["result"].GetObject();
		string res_type = res["type"].GetString();
		if (string("undefined") == res_type )
		{
			return "undefined";
		}
		else if (  string("object") == res_type)
		{
			//create object and ownproperties values
			auto obj = res["preview"]["ownProperties"].GetObject();
			rapidjson::Document res_doc;
			res_doc.SetObject();
			auto it = obj.MemberBegin(); auto end = obj.MemberEnd();
			for (; it != end; ++it)
			{
				res_doc.AddMember(it->name, it->value.GetObject()["value"], res_doc.GetAllocator());
			}

			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			res_doc.Accept(writer);

			return buffer.GetString();
		}
	}
	else
	{

	}
#endif

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	
	valOfdoc["result"].Accept(writer);

	return buffer.GetString();



}

void FireFoxDriver::AttachTab(const Tab& inTab, function<void(const string&)>&& inCB)
{
	string msg = "{\"to\":\":actor\", \"type\": \"attach\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab.GetActor());
	auto reply = _SendRequest(msg);
	while (true)
		reply = _ReadOneJSONPacket();

	return;
}

asio::io_context & FireFoxDriver::GetIOService()
{
	return m_ioservice;
}

void FireFoxDriver::OnPacketRecevied(const JSONPacket &packet)
{
	cout << packet.GetMsg() << endl;

	rapidjson::Document document;
	if (document.Parse(packet.GetMsg()).HasParseError())
	{
		std::cerr << "Error while Parsing recieved JSON:  " << document.GetParseError() << std::endl;
	}
	else
	{
		auto obj = document.GetObject();
		//check actor
		string actor = obj["from"].GetString();
		/*if (actor == "root" && obj.HasMember("tabs"))
		{
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
		}*/
		if (m_status == eWaitingHandShake)
		{
			m_status = eReady;
			m_onConnectHandler();
		}
		else
		{
			m_activeRequests[actor](packet);
		}
		
	}


}

void FireFoxDriver::Run()
{
	
	m_ioservice.run();
}

void FireFoxDriver::OnConnect(function<void(void)>&& inCB)
{
	// become ready
	m_onConnectHandler = inCB;
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
