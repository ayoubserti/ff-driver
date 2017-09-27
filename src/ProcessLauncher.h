/*
    {{license}}
*/

#ifndef __PROCESS_LAUNCHR_H__
#define __PROCESS_LAUNCHR_H__

#include <windows.h>
#include <string>

class FirefoxProcess
{
    PROCESS_INFORMATION  m_processInfo;
	void Close();
    public: 
    FirefoxProcess(const std::string& optArgs = "");
	virtual ~FirefoxProcess();
    
};

#endif //__PROCESS_LAUNCHR_H__