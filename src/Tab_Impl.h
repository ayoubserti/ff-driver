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
	mutable std::string m_TabThreadActor; //TODO: find more elegant way

	friend class FireFoxDriver_Impl;


	
public:

	Tab_Impl() : 
		Tab(this)
		,m_TabURL("")
	    ,m_title("")
	    ,m_tabActor("")
		,m_consoleActor("")
		,m_TabThreadActor("")
	{}

	

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
	const std::string& GetThreadActor() const
	{
		return m_TabThreadActor;
	}
	void  SetThreadActor(const std::string& actor) const
	{
		m_TabThreadActor = actor;
	}
	Tab* Clone() const
	{
		return new Tab_Impl(*this);
	}

};

#endif 