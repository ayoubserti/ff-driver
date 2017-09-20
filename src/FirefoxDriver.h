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

#include <string>
#include <vector>

#include "ProcessLauncher.h"
#include "JSONPacket.h"

#include <functional>


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


class FireFoxDriver : public FirefoxProcess {

    
    FireFoxDriver(const FireFoxDriver& ) = delete;
    FireFoxDriver( FireFoxDriver&& ) = delete;
    
	asio::io_service	m_ioservice;
	asio::ip::tcp::socket m_endpoint;

	JSONPacket  _ReadOneJSONPacket();

	JSONPacket  _SendRequest(const string& msg);

	static vector<string>  s_unsolicitedEvents;

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
		@params
				-inTab tab to attach
				-inCB function to execute for every event comming
		@return void

	*/

	void	AttachTab(const Tab& inTab, function<void(const string&)>&& inCB);
    
};


#endif //__FIREFOXDRIVER_H__