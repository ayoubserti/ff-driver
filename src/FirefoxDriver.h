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
#include <memory>

#define ASYNC_API 1
using namespace std;

//forward declation



class FireFoxDriver;

class Tab{

    string m_TabURL;
    string m_title;
    string m_tabActor;
	string m_consoleActor;
	mutable	string m_ThreadActor;
	mutable string m_PauseActor;

	friend class FireFoxDriver;

	
	void  _InterceptAttachTab(const JSONPacket& packet, function<void(const JSONPacket&)> &&inCB) const;

    public:

	
	
	enum eThreadState
	{
		eDetached,
		ePaused,
		eRunning,
		eExited

	};
	Tab();
    string GetURL() const;
    string GetTitle() const;
    string GetActor() const;
	string GetConsoleActor() const;
	string GetThreadActor() const;
	string GetPauseActor() const;

	eThreadState GetTabThreadState() const;
private:
	mutable eThreadState  m_TabThreadState;
};


class Source
{
public:
	string m_sourceActor;
	string m_url;
	
};



class SourceLocation
{

	string m_URL;
		long m_Line,
		m_Column;

public:

	string GetURL() const;
	long  GetLine() const;
    long GetColumn() const;

	void SetURL(const string& inURL);
	void SetLine(long inLine);
	void SetColumn(long inColumn);

};

struct Request {

private:

	Request();
public:

	JSONPacket m_packet{ "" };
	function<void(const JSONPacket&) > m_callback;
	string		m_to{ "" };

	Request(const string& to,const JSONPacket& packet, function<void(const JSONPacket&) >&& cb)
	{
		m_to = to;
		m_packet = packet;
		m_callback = cb;
	}

};


class FireFoxDriver : public FirefoxProcess ,public INetworkDelegate {

    
    FireFoxDriver(const FireFoxDriver& ) = delete;
    FireFoxDriver( FireFoxDriver&& ) = delete;
    
	asio::io_service	m_ioservice;

	
	static vector<string>  s_unsolicitedEvents;

	typedef function<void(const JSONPacket&)>&&  CallBackType;
	
	/*
		while we are in asynchronous mode and Debugger Server doesn't tag request to Actors
		every Actor can only have one active Request

	*/
	vector<shared_ptr<Request>> m_pendingRequests;

	map<string, shared_ptr<Request> >   m_activeRequests;

	
	function<void(void)>  m_onConnectHandler;
	
	enum  DriverState {
		eStop,
		eWaitingHandShake,
		eReady
	};

	DriverState m_status;

	Endpoint  m_asyncEndpoint;
	
	void	_prepareToSend(const string& packet);

public:
    
    /*
        @ctor
		@params
				optArgs  optional argument to be passed to FireFox
        @info Create new Firefox process
    */
	FireFoxDriver(const string& optArgs = "");
    
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

    void     OpenNewTab(const string& url, CallBackType inCB);


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

	void EvaluateJS(const Tab& inTab, const string& inScript, CallBackType inCB);

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

	bool    AttachTabThread(const Tab& inTab, CallBackType inCB);

	bool	ResumeThread(const Tab& inTab, CallBackType inCB);

	bool    SetBreakPoint(const Tab& inTab, const SourceLocation& sourceLocation,CallBackType inCB);

	void	GetSourceOfTab(const Tab& inTab, function<void(const vector<Source>&)>&& inCB);

	void	GetSourceCode(const Source& inSource, function<void(const string&)>&& inCB);

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


#endif //__FIREFOXDRIVER_H__