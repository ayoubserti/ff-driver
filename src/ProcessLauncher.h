/*
    {{license}}
*/

#ifndef __PROCESS_LAUNCHR_H__
#define __PROCESS_LAUNCHR_H__

#include <windows.h>

class FirefoxProcess
{
    PROCESS_INFORMATION  m_processInfo;

    public: 
    FirefoxProcess();
	virtual ~FirefoxProcess();
    
	void Close();
    

};

#endif //__PROCESS_LAUNCHR_H__