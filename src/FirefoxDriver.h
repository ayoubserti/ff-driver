/*
    {{license}}
*/

#ifndef __FIREFOXDRIVER_H__
#define __FIREFOXDRIVER_H__

#ifdef _WIN32
#define  _WIN32_WINNT  0x0601
#define  ASIO_DISABLE_IOCP 1
#endif
#define  ASIO_STANDALONE 1
#include "asio.hpp"

#include "Network.h"
#include <string>
#include <vector>

#include "ProcessLauncher.h"
#include "JSONPacket.h"
#include <map>

#include <functional>

#define ASYNC_API 1
using namespace std;

//forward declation

class FireFoxDriver;
class Tab{

    string m_TabURL;
    string m_title;
    string m_tabActor;
	string m_consoleActor;
	friend class FireFoxDriver;

    public:


    string GetURL() const;
    string GetTitle() const;
    string GetActor() const;
	string GetConsoleActor() const;
};


struct Request {

private:

	Request() = delete;
public:

	JSONPacket m_packet{ "" };
	function<void(const JSONPacket&) > m_callback;
	string		m_to{ "" };

	Request(const string& to,const JSONPacket& packet, function<void(const JSONPacket&) >&& cb) {
		m_to = to;
		m_packet = packet;
		m_callback = cb;
	}

	

};


class FireFoxDriver : public FirefoxProcess ,public INetworkDelegate {

    
    FireFoxDriver(const FireFoxDriver& ) = delete;
    FireFoxDriver( FireFoxDriver&& ) = delete;
    
	asio::io_service	m_ioservice;

	asio::ip::tcp::socket m_endpoint;

	JSONPacket  _ReadOneJSONPacket();

	JSONPacket  _SendRequest(const string& msg);

	static vector<string>  s_unsolicitedEvents;
	
	/*
		while we are in asynchronous mode and Debugger Server doesn't tag request to Actors
		every Actor can only have one active Request

	*/

	vector<Request> m_pendingRequests;

	map<string, Request >   m_activeRequests;

	

	
	function<void(void)>  m_onConnectHandler;
	
	enum  DriverState {
		eStop,
		eWaitingHandShake,
		eReady
	};

	DriverState m_status;
	

#if ASYNC_API
	Endpoint  m_asyncEndpoint;
#endif

	
	void	_prepareToSend(const JSONPacket& packet);

    public:
    
    /*
        @ctor
        @info Create new Firefox process
    */
    FireFoxDriver();
    
    /*
        @function GetTabList
        @info     return vector of opened tab
        @return   vector of Tab object
    */
    std::vector<Tab> GetTabList();

	//asynchronous version
	void GetTabList(function<void(const vector<Tab>&)>&& inCB);
    
    /*
        @function OpenNewTab
        @info    open new tab in the current firefox instance
		@params  url to navigate to
        @warning   
    */

    Tab     OpenNewTab(const string& url );


    /*
        @function NavigateTo
    */
    void    NavigateTo(const Tab& inTab, const std::string& inUrl);
    
	void	NavigateTo(const Tab& inTab, const std::string& inUrl, function<void(const JSONPacket&)> && inCB);
	

    /*
        @function   CloseTab
    */
    void CloseTab(const Tab& inTab);

    /*
     @function      ReloadTab
    */

    void ReloadTab(const Tab& inTab);

	/*
	@function EvaluteJS
	@params  
			-inTab tab where the javascript code will be evaluated
			-inScript  script to evaluate
	@return 
			return result as text
	*/

	const string EvaluateJS(const Tab& inTab, const string& inScript);

	/*
		@function  AttachTab
		@info	   attach to Tab and subscribe to events; 
				   this function is asynchronous
		@params
				-inTab tab to attach
				-inCB function to execute for every event comming
		
		@return void

	*/

	void	AttachTab(const Tab& inTab, function<void(const JSONPacket&)>&& inCB);

	//from INetworkDelegate

	asio::io_context& GetIOService() override;

	void OnPacketRecevied(const JSONPacket&) override;

    void OnPacketSend(const JSONPacket&) override;


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


#endif //__FIREFOXDRIVER_H__