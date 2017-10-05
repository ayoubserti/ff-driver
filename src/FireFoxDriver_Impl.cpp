
#include "FireFoxDriver_Impl.h"
#include "Tab_Impl.h"


#define RAPIDJSON_HAS_STDSTRING 1
#if _WIN32
#undef GetObject
#endif
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"




#include <string>
#include <iostream>
#include <regex>

using namespace asio::ip;
using namespace std;


class Request {

private:

	Request();
public:

	JSONPacket_Impl m_packet{ "" };
	function<void(const JSONPacket&) > m_callback;
	std::string		m_to{ "" };

	Request(const std::string& to, const JSONPacket& packet, function<void(const JSONPacket&) >&& cb)
		:m_to(to),
		m_packet(dynamic_cast<const JSONPacket_Impl&>(packet)),
		m_callback(cb)
	{

	}

};



FireFoxDriver_Impl::FireFoxDriver_Impl(const std::string& optArgs) :
	FireFoxDriver(),
	FirefoxProcess(optArgs)
	, m_asyncEndpoint(*this, "127.0.0.1", 6000)
{
	m_asyncEndpoint.Start();
	m_status = eWaitingHandShake;
}
void FireFoxDriver_Impl::GetTabList(function<void(const vector<Tab*>&)>&& inCB)
{

	JSONPacket_Impl jsonPacket("{ \"to\":\"root\", \"type\":\"listTabs\" }");
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
			vector<Tab*> tabs;
			const auto& tabsArray = obj["tabs"].GetArray();
			for (auto& it : tabsArray)
			{
				auto tabObj = it.GetObject();
				Tab_Impl* tab= new Tab_Impl;
				tab->m_title = tabObj["title"].GetString();
				tab->m_tabActor = tabObj["actor"].GetString();
				tab->m_TabURL = tabObj["url"].GetString();
				tab->m_consoleActor = tabObj["consoleActor"].GetString();
				tabs.push_back(tab);
			}
			inCB(tabs);
		}
	};

	shared_ptr<Request> req(new Request("root", jsonPacket, completion));
	m_pendingRequests.push_back(req);
	_prepareToSend("root");
}


void FireFoxDriver_Impl::OpenNewTab(const string& url, CallBackType inCB)
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

void FireFoxDriver_Impl::NavigateTo(const Tab& inTab, const std::string & inUrl, function<void(const JSONPacket&)>&& inCB)
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

	JSONPacket_Impl json(buffer.GetString());
	string tabActor = inTab.GetActor();
	shared_ptr<Request> req(new Request(tabActor, json, std::move(inCB)));
	m_pendingRequests.push_back(req);
	_prepareToSend(tabActor);

}

void FireFoxDriver_Impl::CloseTab(const Tab& inTab, CallBackType inCB)
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

	JSONPacket_Impl json(buffer.GetString());
	shared_ptr<Request> req(new Request(inTab.GetActor(), json, std::move(inCB)));
	m_pendingRequests.push_back(req);
	_prepareToSend(inTab.GetActor());
}

void FireFoxDriver_Impl::ReloadTab(const Tab& inTab, CallBackType inCB)
{
	string msg = "{\"to\":\":actor\", \"type\": \"reload\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab.GetActor());
	JSONPacket_Impl json(msg);

	shared_ptr<Request>  req(new Request(inTab.GetActor(), json, std::move(inCB)));
	m_pendingRequests.push_back(req);
	_prepareToSend(inTab.GetActor());

}


void FireFoxDriver_Impl::EvaluateJS(const Tab& inTab, const string& inScript, CallBackType inCB)
{
	rapidjson::Document doc;
	doc.SetObject();
	doc.AddMember("to", inTab.GetConsoleActor(), doc.GetAllocator());
	doc.AddMember("type", "evaluateJS", doc.GetAllocator());
	doc.AddMember("text", inScript, doc.GetAllocator());

	JSONPacket_Impl reply("");
	//serialiaze msg
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);


		JSONPacket_Impl json(buffer.GetString());
		shared_ptr<Request> req(new Request(inTab.GetConsoleActor(), json, std::move(inCB)));
		m_pendingRequests.push_back(req);
		_prepareToSend(inTab.GetConsoleActor());

	}

}

void FireFoxDriver_Impl::AttachTab(const Tab& inTab, function<void(const JSONPacket&)>&& inCB)
{
	string msg = "{\"to\":\":actor\", \"type\": \"attach\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab.GetActor());
	JSONPacket_Impl request(msg);

	
	shared_ptr<Request> req(new Request(inTab.GetActor(), request, std::move(inCB)));

	m_pendingRequests.push_back(req);
	_prepareToSend(inTab.GetActor());

}


void FireFoxDriver_Impl::AttachTab(Tab* inTab, function<void(const JSONPacket&)>&& inCB)
{
	string msg = "{\"to\":\":actor\", \"type\": \"attach\"}";
	msg = std::regex_replace(msg, regex(":actor"), inTab->GetActor());
	JSONPacket_Impl request(msg);

	Tab_Impl* tab = dynamic_cast< Tab_Impl*>(inTab);

	auto interceptor = [=](const JSONPacket& packet)
	{
		//need an interceptor to add ThreadActor
		if (tab->GetThreadActor() == "")
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
				if (obj.HasMember("from") && (tab->GetActor() == obj["from"].GetString()))
				{
					if (obj.HasMember("type") && (string("tabAttached") == obj["type"].GetString()))
					{
						tab->SetThreadActor(obj["threadActor"].GetString());
					}
				}
			}
		}

		inCB(packet);

	};

	shared_ptr<Request> req(new Request(inTab->GetActor(), request, std::move(interceptor)));

	m_pendingRequests.push_back(req);
	_prepareToSend(inTab->GetActor());

}
bool    FireFoxDriver_Impl::AttachTabThread(const Tab& inTab, CallBackType inCB)
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
		JSONPacket_Impl request(msg);

		shared_ptr<Request> req(new Request(actor, request, std::move(inCB)));

		m_pendingRequests.push_back(req);
		_prepareToSend(actor);

		return true;
	}
}

asio::io_context & FireFoxDriver_Impl::GetIOService()
{
	return m_ioservice;
}

void FireFoxDriver_Impl::OnPacketRecevied(const JSONPacket &packet)
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
		std::string actor = obj["from"].GetString();

		if (m_status == eWaitingHandShake)
		{
			m_status = eReady;
			m_onConnectHandler();
		}
		else
		{
			if (m_activeRequests.find(actor) != m_activeRequests.end())
				m_activeRequests[actor]->m_callback(packet);

			_prepareToSend(actor);
		}

	}


}

void FireFoxDriver_Impl::_prepareToSend(const std::string& actor)
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

void FireFoxDriver_Impl::Run()
{

	m_ioservice.run();
}

void FireFoxDriver_Impl::OnConnect(function<void(void)>&& inCB)
{
	// become ready
	m_onConnectHandler = inCB;
}

void FireFoxDriver_Impl::Stop()
{
	m_ioservice.stop();
}

static const std::string emptyString("");

Tab::Tab()
	:m_impl(nullptr),
	m_TabThreadState(eThreadState::eDetached)
{

}

Tab::Tab(Tab_Impl* impl)
	:m_impl(impl),
	m_TabThreadState(eThreadState::eDetached)
{}

Tab::Tab(const Tab_Impl& other)
{
	m_impl = &(const_cast<Tab_Impl&>(other));
}

const string& Tab::GetURL() const
{
	//if (m_impl != nullptr) return m_impl->GetURL();
	return emptyString;
}

const string& Tab::GetTitle() const
{
	//if (m_impl != nullptr) return m_impl->GetTitle();
	return emptyString;
}

const string& Tab::GetActor() const
{
	if (m_impl != nullptr) return m_impl->GetActor();
	return emptyString;
}

const string& Tab::GetConsoleActor() const
{
	//if (m_impl != nullptr) return m_impl->GetConsoleActor();
	return emptyString;
}

const string& Tab::GetThreadActor() const
{
	return emptyString;
}
Tab::eThreadState Tab::GetTabThreadState() const
{
	return m_TabThreadState;
}

void Tab::SetThreadActor(const std::string&) const
{
	
}
Tab*  Tab::Clone() const
{
	return nullptr;
}



Tab::~Tab()
{
	
}