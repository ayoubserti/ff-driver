#ifndef __FIREFOX_IMPL_H__
#define __FIREFOX_IMPL_H__

#include "Network.h"
#include "Network_Impl.h"

#include "FirefoxDriver.h"
#include "ProcessLauncher.h"


#include "JSONPacket.h"


#include <map>
#include <string>
#include <functional>
#include <memory>


using namespace std;

class  FireFoxDriver_Impl : public FireFoxDriver,  public FirefoxProcess, public INetworkDelegate {


	FireFoxDriver_Impl(const FireFoxDriver_Impl&) = delete;
	FireFoxDriver_Impl(FireFoxDriver_Impl&&) = delete;

	asio::io_service	m_ioservice;


	/*
	while we are in asynchronous mode and Debugger Server doesn't tag request to Actors
	every Actor can only have one active Request

	*/
	vector<shared_ptr<Request>> m_pendingRequests;

	map<std::string, shared_ptr<Request> >   m_activeRequests;


	std::function<void(void)>  m_onConnectHandler;

	enum  DriverState {
		eStop,
		eWaitingHandShake,
		eReady
	};

	DriverState m_status;

	Endpoint_Impl  m_asyncEndpoint;

	void	_prepareToSend(const std::string& packet);

public:

	/*
	@ctor
	@params
	optArgs  optional argument to be passed to FireFox
	@info Create new Firefox process
	*/
	FireFoxDriver_Impl(const std::string& optArgs = "");

	/*
	@function GetTabList
	@info     return vector of opened tab
	@params
	-inCB callback function
	@return   vector of Tab object
	*/


	void GetTabList(function<void(const vector<Tab>&)>&&  inCB);

	/*
	@function OpenNewTab
	@info    open new tab in the current firefox instance
	@params
	-url to navigate to
	-inCB callback function
	@warning
	*/

	void     OpenNewTab(const std::string& url, CallBackType inCB);


	/*
	@function NavigateTo
	*/

	void	NavigateTo(const Tab& inTab, const std::string& inUrl, CallBackType && inCB);


	/*
	@function   CloseTab
	*/
	void CloseTab(const Tab& inTab, CallBackType inCB);

	/*
	@function      ReloadTab
	*/

	void ReloadTab(const Tab& inTab, CallBackType inCB);

	/*
	@function EvaluteJS
	@params
	-inTab tab where the javascript code will be evaluated
	-inScript  script to evaluate
	-inCB	   callback function

	@return
	return result as text
	*/

	void EvaluateJS(const Tab& inTab, const std::string& inScript, CallBackType inCB);

	/*
	@function  AttachTab
	@info	   attach to Tab and subscribe to events;
	this function is asynchronous
	@params
	-inTab tab to attach
	-inCB function to execute for every event comming

	@return void

	*/

	void	AttachTab(const Tab& inTab, CallBackType inCB);

	//from INetworkDelegate

	asio::io_context& GetIOService() override;

	void OnPacketRecevied(const JSONPacket&) override;


	/*
	@function	Run
	@info		run underlaying io_service for aynchronous operation
	it should be the last line of code because it will block
	*/
	void Run();



	/*
	@function OnConnect
	@info		install the callback function to be executed when The driver is connected to FF
	*/
	void OnConnect(function<void(void)>&&);


	/*
	@function	Stop
	@info		stop event polling, the main program continue
	@warning	doesn't close socket with FF
	*/

	void Stop();


};

#endif