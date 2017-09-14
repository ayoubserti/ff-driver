/*
    {{license}}
*/

#include "ProcessLauncher.h"

FirefoxProcess::FirefoxProcess()
{

    //start Firefox Process
    //we assume we are on Windows (7- ) platform with firefox installed on the usual place
    //which is C:\Program Files (x86)\Mozilla Firefox


	STARTUPINFO startupInfo; 

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof startupInfo; 
	BOOL bCreated = CreateProcess(NULL,"C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe -start-debugger-server 6000", NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &m_processInfo);
	if (bCreated) {
		//successfully created; wait to start
		WaitForSingleObject(m_processInfo.hProcess, 3000); //wait 3seconds
	}
	else {
		//Todo throw exception
	}

	
}

FirefoxProcess::~FirefoxProcess()
{
	Close();
}

void FirefoxProcess::Close()
{
	
	CloseHandle(m_processInfo.hThread); CloseHandle(m_processInfo.hProcess);
	
}
