#ifndef __TAB_IMPL_H__
#define __TAB_IMPL_H__

#include "FirefoxDriver.h"
#include <string>
using namespace std;

class Tab_Impl : public Tab
{

	std::string m_TabURL;
	std::string m_title;
	std::string m_tabActor;
	std::string m_consoleActor;

	friend class FireFoxDriver_Impl;
	
public:

	Tab_Impl() : Tab(this) {}

	const string& GetURL() const
	{
		return m_TabURL;
	}
	const string& GetTitle() const
	{
		return m_title;
	}
	const string& GetActor() const {
		return m_tabActor;
	}
	const string& GetConsoleActor() const
	{
		return m_consoleActor;
	}

};

#endif 