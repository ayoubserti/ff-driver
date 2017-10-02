#define RAPIDJSON_HAS_STDSTRING 1
#include "FirefoxDriver.h"
#include "Tab_Impl.h"
#if _WIN32
#undef GetObject
#endif
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "FireFoxDriver_Impl.h"

#include <iostream>
#include <regex>

using namespace asio::ip;



FireFoxDriver* FireFoxDriver::Create(const std::string& optArgs)
{
	return new FireFoxDriver_Impl(optArgs);
}


void FireFoxDriver::GetTabList(function<void(const vector<Tab*>&)>&& inCB)
{

	//m_impl->GetTabList(std::move(inCB));
}
void FireFoxDriver::GetTabList(function<void(const vector<Tab>&)>&& inCB)
{

	//m_impl->GetTabList(std::move(inCB));
}
void FireFoxDriver::OpenNewTab(const string& url, CallBackType inCB)
{
	//m_impl->OpenNewTab(url, std::move(inCB));
}

void FireFoxDriver::NavigateTo(const Tab & inTab, const std::string & inUrl, function<void(const JSONPacket&)>&& inCB)
{
	
	//m_impl->NavigateTo(inTab, inUrl, std::move(inCB));
}

void FireFoxDriver::CloseTab(const Tab & inTab, CallBackType inCB)
{
	//m_impl->CloseTab(inTab, std::move(inCB));
}

void FireFoxDriver::ReloadTab(const Tab & inTab, CallBackType inCB)
{
	//m_impl->ReloadTab(inTab, std::move(inCB));
	
}


void FireFoxDriver::EvaluateJS(const Tab& inTab, const string& inScript, CallBackType inCB)
{
	//m_impl->EvaluateJS(inTab, inScript, std::move(inCB));
}

void FireFoxDriver::AttachTab(const Tab& inTab, function<void(const JSONPacket&)>&& inCB)
{
	//m_impl->AttachTab(inTab, std::move(inCB));

}

void FireFoxDriver::Run()
{
	
	//m_impl->Run();
}

void FireFoxDriver::OnConnect(function<void(void)>&& inCB)
{
	//m_impl->OnConnect(std::move(inCB));
}

void FireFoxDriver::Stop()
{
	//m_impl->Stop();
}

