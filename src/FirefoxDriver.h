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
	string m_consoleActor;
	friend class FireFoxDriver;

    public:


    string GetURL() const;
    string GetTitle() const;
    string GetActor() const;
	string GetConsoleActor() const;
};


class JSONPacket {
	string m_msg;
public:
	JSONPacket(const string& msg) : m_msg(msg) {}
	string Stringify() {
		string result;
		result += std::to_string(m_msg.size());
		result += ":";
		result += m_msg;
		return result;
	}

	static JSONPacket Parse(const string& jsonmsg) {

		JSONPacket packet("");
		size_t offset = jsonmsg.find(":");
		if (offset>0)
		{
			size_t packetlen = std::stol(jsonmsg.substr(0, offset + 1));
			packet.m_msg = jsonmsg.substr(offset + 1,packetlen); //be sure we get only one single packet form msg

		}

		return packet;

	}

	string GetMsg()
	{
		return m_msg;
	}

	const string& GetMsg() const
	{
		return m_msg;
	}

};


class FireFoxDriver : public FirefoxProcess {

    
    FireFoxDriver(const FireFoxDriver& ) = delete;
    FireFoxDriver( FireFoxDriver&& ) = delete;
    
	asio::io_service	m_ioservice;
	asio::ip::tcp::socket m_endpoint;

	JSONPacket  _ReadOneJSONPacket();

	JSONPacket  _SendRequest(const string& msg);

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
        @warning   
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