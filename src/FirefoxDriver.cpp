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


FireFoxDriver::FireFoxDriver(const string& optArgs ) :
	FirefoxProcess(optArgs)
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
	_prepareToSend("root");
}

void FireFoxDriver::OpenNewTab(const string& url, CallBackType inCB)
{
	//I decide to trop OpenNewTab api because isn't natively supported and the current implementation may have side effect
	
	/*auto completion = [&]( const vector<Tab>& tabs) {

		if (tabs.size())
		{
			rapidjson::Document doc;
			doc.SetObject();
			doc.AddMember("to", tabs[0].GetConsoleActor(), doc.GetAllocator());
			doc.AddMember("type", "evaluateJS", doc.GetAllocator());
			string scriptText = "window.open('";
			scriptText += url;
			scriptText += "')";
			doc.AddMember("text", scriptText, doc.GetAllocator());

			//serialiaze msg
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			doc.Accept(writer);
			JSONPacket json(buffer.GetString());
			string tabActor = tabs[0].GetConsoleActor();
			shared_ptr<Request> req(new Request(tabActor, json, std::move(inCB)));
			m_pendingRequests.push_back(req);
			_prepareToSend(tabActor);
		}
	};


	GetTabList(completion);*/
	
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
	_prepareToSend(tabActor);
	
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
	m_pendingRequests.push_back(req);
	_prepareToSend(inTab.GetActor());
}

void FireFoxDriver::ReloadTab(const Tab & inTab, CallBackType inCB)
{
	string msg = "{\"to\":\":actor\", \"type\": \"reload\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab.GetActor());
	JSONPacket json(msg);

	shared_ptr<Request>  req(new Request(inTab.GetActor(), json, std::move(inCB)));
	m_pendingRequests.push_back(req);
	_prepareToSend(inTab.GetActor());
	
}


void FireFoxDriver::EvaluateJS(const Tab& inTab, const string& inScript, CallBackType inCB)
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


		JSONPacket json(buffer.GetString());
		shared_ptr<Request> req(new Request(inTab.GetConsoleActor(), json, std::move(inCB)));
		m_pendingRequests.push_back(req);
		_prepareToSend(inTab.GetConsoleActor());

	}
	
}

void FireFoxDriver::AttachTab(const Tab& inTab, function<void(const JSONPacket&)>&& inCB)
{
	string msg = "{\"to\":\":actor\", \"type\": \"attach\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab.GetActor());
	JSONPacket request(msg);

	auto interceptor = [&inTab,inCB](const JSONPacket& packet) {
		//need an interceptor to add ThreadActor
		if (inTab.GetThreadActor() == "")
		{
			rapidjson::Document document;
			if (document.Parse(packet.GetMsg()).HasParseError())
			{
				std::cerr << "Error while Parsing recieved JSON:  " << document.GetParseError() << std::endl;
				return;
			}
			else
			{
				auto obj = document.GetObject();
				if (obj.HasMember("from") && (inTab.GetActor() == obj["from"].GetString()))
				{
					if (obj.HasMember("type") && (string("tabAttached") == obj["type"].GetString()))
					{
						inTab.m_ThreadActor = obj["threadActor"].GetString();
					}
				}
			}
		}
		inCB(packet);
	};

	shared_ptr<Request> req(new Request(inTab.GetActor(), request, std::move(interceptor)));

	m_pendingRequests.push_back(req);
	_prepareToSend(inTab.GetActor());

}
bool    FireFoxDriver::AttachTabThread(const Tab& inTab, function<void(const JSONPacket&)>&& inCB)
{
	if (inTab.GetTabThreadState() != Tab::eThreadState::eDetached)
	{
		return false;
	}
	else
	{
		string msg = "{\"to\":\":actor\", \"type\": \"attach\"}";
		string actor = inTab.GetThreadActor();
		msg = std::regex_replace(msg, regex(":actor"), actor);
		JSONPacket request(msg);

		auto interceptor = [&inTab, inCB](const JSONPacket& packet) {
			
			inTab.m_TabThreadState = Tab::ePaused;
			inCB(packet);
		};

		shared_ptr<Request> req(new Request(actor, request, std::move(interceptor)));

		m_pendingRequests.push_back(req);
		_prepareToSend(actor);

		return true;
	}
}


bool FireFoxDriver::SetBreakPoint(const Tab & inTab, const SourceLocation & sourceLocation, CallBackType inCB)
{
	if((inTab.GetTabThreadState() != Tab::ePaused) && (inTab.GetTabThreadState() != Tab::eRunning))
	   return false;

	string msg = "{\"to\": \":actor\", \"type\": \"setBreakpoint\" , \"location\" : {\"url\":\":urlplace\", \"line\": :lineplace , \"column\" : :columnplace}}";
	string actor = inTab.GetThreadActor();
	msg = std::regex_replace(msg, regex(":actor"), actor);
	msg = regex_replace(msg, regex(":urlplace"), sourceLocation.GetURL());
	msg = regex_replace(msg, regex(":lineplace"), std::to_string(sourceLocation.GetLine()));
	msg = regex_replace(msg, regex(":columnplace"), std::to_string(sourceLocation.GetColumn()));
	JSONPacket request(msg);

	shared_ptr<Request> req(new Request(actor, request, std::move(inCB)));

	m_pendingRequests.push_back(req);
	_prepareToSend(actor);

	return true;

}
asio::io_context & FireFoxDriver::GetIOService()
{
	return m_ioservice;
}

void FireFoxDriver::OnPacketRecevied(const JSONPacket &packet)
{
	
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
			if( m_activeRequests.find(actor) != m_activeRequests.end())
				m_activeRequests[actor]->m_callback(packet);
			
			_prepareToSend(actor);
		}
		
		
	}


}

void FireFoxDriver::_prepareToSend(const string& actor)
{
	string to = actor;

	vector<string> elemToShrink;
	
	for (auto& it : m_pendingRequests)
	{
		if (m_activeRequests.find(it->m_to) == m_activeRequests.end()) {
			elemToShrink.push_back(it->m_to);
			m_asyncEndpoint.Send(it->m_packet);
			m_activeRequests[it->m_to] = it;
			
		}
		else
		{
			m_activeRequests.erase(it->m_to);
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

Tab::Tab()
	:m_TabThreadState(eDetached)
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

string Tab::GetConsoleActor() const
{
	return m_consoleActor;
}

string Tab::GetThreadActor() const
{
	return m_ThreadActor;
}

Tab::eThreadState Tab::GetTabThreadState() const
{
	return m_TabThreadState;
}

void Tab::_InterceptAttachTab(const JSONPacket& packet, function<void(const JSONPacket&)> &&inCB) const
{
	
	//need an interceptor to add ThreadActor
	if (GetThreadActor() == "")
	{
		rapidjson::Document document;
		if (document.Parse(packet.GetMsg()).HasParseError())
		{
			std::cerr << "Error while Parsing recieved JSON:  " << document.GetParseError() << std::endl;
			return;
		}
		else
		{
			auto obj = document.GetObject();
			if (obj.HasMember("from") && (GetActor() == obj["from"].GetString()))
			{
				if (obj.HasMember("type") && (string("tabAttached") == obj["type"].GetString()))
				{
					m_ThreadActor = obj["threadActor"].GetString();
				}
			}
		}
	}
	inCB(packet);
}

string SourceLocation::GetURL() const
{
	return m_URL;
}

long SourceLocation::GetLine() const
{
	return m_Line;
}

long SourceLocation::GetColumn() const
{
	return m_Column;
}

void SourceLocation::SetURL(const string & inURL)
{
	m_URL = inURL;
}

void SourceLocation::SetLine(long inLine)
{
	m_Line = inLine;
}

void SourceLocation::SetColumn(long inColumn)
{
	m_Column = inColumn;
}
