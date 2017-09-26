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

using namespace asio::ip;


vector<string> FireFoxDriver::s_unsolicitedEvents = {"tabListChanged"};


FireFoxDriver::FireFoxDriver() :
	FirefoxProcess()
	,m_asyncEndpoint(*this,"127.0.0.1",6000)
{	
	m_asyncEndpoint.Start();
	m_status = eWaitingHandShake;
}
void FireFoxDriver::GetTabList(function<void(const vector<Tab>&)>&& inCB)
{

	JSONPacket jsonPacket("{ \"to\":\"root\", \"type\":\"listTabs\" }");
	auto completion = [=](const JSONPacket& packet) {

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

	shared_ptr<Request> req(new Request("root", jsonPacket, completion));
	m_pendingRequests.push_back(req);
	_prepareToSend("to");
}

void FireFoxDriver::OpenNewTab(const string& url, CallBackType inCB)
{
	/*Tab tab;
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
	*/
}

void FireFoxDriver::NavigateTo(const Tab & inTab, const std::string & inUrl, function<void(const JSONPacket&)>&& inCB)
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
	string tabActor = inTab.GetActor();
	shared_ptr<Request> req(new Request(tabActor, json, std::move(inCB)));
	m_pendingRequests.push_back(req);
	
	
}

void FireFoxDriver::CloseTab(const Tab & inTab, CallBackType inCB)
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
	shared_ptr<Request> req(new Request(inTab.GetActor(), json, std::move(inCB)));

}

void FireFoxDriver::ReloadTab(const Tab & inTab, CallBackType inCB)
{
	string msg = "{\"to\":\":actor\", \"type\": \"reload\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab.GetActor());
	
}


void FireFoxDriver::EvaluateJS(const Tab& inTab, const string& inScript, CallBackType inCB)
{
	/*rapidjson::Document doc;
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
	*/



}

void FireFoxDriver::AttachTab(const Tab& inTab, function<void(const JSONPacket&)>&& inCB)
{
	string msg = "{\"to\":\":actor\", \"type\": \"attach\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab.GetActor());
	JSONPacket request(msg);

	shared_ptr<Request> req(new Request(inTab.GetActor(), request, std::move(inCB)));

	m_pendingRequests.push_back(req);
	_prepareToSend(inTab.GetActor());

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
		
		if (m_status == eWaitingHandShake)
		{
			m_status = eReady;
			m_onConnectHandler();
		}
		else
		{
			m_activeRequests[actor]->m_callback(packet);
			
			
		}
		_prepareToSend(actor);
		
	}


}

void FireFoxDriver::_prepareToSend(const string& actor)
{
	string to = actor;

	vector<string> elemToShrink;
	
	for (auto& it : m_pendingRequests)
	{
		if (m_activeRequests.find(it->m_to) != m_activeRequests.end()) {
			elemToShrink.push_back(it->m_to);
			
		}
		else
		{
			m_asyncEndpoint.Send(it->m_packet);
			m_activeRequests[it->m_to] = it;
		}
		
	}

	std::remove_if(m_pendingRequests.begin(), m_pendingRequests.end(), [&](const shared_ptr<Request> req) {
		if (find(elemToShrink.begin(), elemToShrink.end(), req->m_to) != elemToShrink.end())
			return true;
		return false;
	});
	
	//remove empty
	size_t t = elemToShrink.size();
	while (t)
	{
		m_pendingRequests.pop_back();
		--t;
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

void FireFoxDriver::Stop()
{
	m_ioservice.stop();
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
