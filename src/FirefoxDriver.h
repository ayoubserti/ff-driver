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



using namespace std;

//forward declation

class FireFoxDriver;
class Tab{

    string m_TabURL;
    string m_title;
    string m_tabActor;

	friend class FireFoxDriver;

    public:


    string GetURL();
    string GetTitle();
    string GetActor();
};




class FireFoxDriver : public FirefoxProcess {

    
    FireFoxDriver(const FireFoxDriver& ) = delete;
    FireFoxDriver( FireFoxDriver&& ) = delete;
    
	asio::io_service	m_ioservice;
	asio::ip::tcp::socket m_endpoint;
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
        @warning    not implemented
    */

    Tab     OpenNewTab();


    /*
        @function NavigateTo
    */
    void    NavigateTo(const Tab& inTab, std::string inUrl);
    

    /*
        @function   CloseTab
    */
    void CloseTab(const Tab& inTab);
    
};


#endif //__FIREFOXDRIVER_H__