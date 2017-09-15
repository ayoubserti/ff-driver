/*
    {{license}}
*/

#include "ProcessLauncher.h"
#include <string>

enum PlatformArch
{
	eX64,
	eX32
};

static std::string GetFireFoxInstallationPathForPlatform(PlatformArch arch)
{
	std::string result="";
	std::string entryPoint;
	if (arch == eX64)
		entryPoint = "SOFTWARE\\Mozilla\\Mozilla Firefox";
	else
		entryPoint = "SOFTWARE\\Wow6432Node\\Mozilla\\Mozilla Firefox";


	HKEY hKey;
	
	LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, entryPoint.c_str(), 0, KEY_READ, &hKey);
	if (lRes == ERROR_SUCCESS)
	{
		//Get version
		char version[20];
		DWORD version_len = sizeof(version);
		lRes = RegQueryValueEx(hKey, "CurrentVersion",0,NULL, (LPBYTE)version, &version_len);
		if (lRes == ERROR_SUCCESS)
		{
			version[version_len] = '\0';
			std::string mozKey = version;
			mozKey += "\\Main";
			HKEY hKey2;
			lRes = RegOpenKeyEx(hKey, mozKey.c_str(), 0, KEY_READ, &hKey2);
			if (lRes == ERROR_SUCCESS)
			{
				char path[MAX_PATH];
				DWORD path_len = sizeof(path);
				lRes = RegQueryValueEx(hKey2, "PathToExe",0,NULL, (LPBYTE)path, &path_len);
				if (lRes == ERROR_SUCCESS)
				{
					result = path;
				}
			}
		}
	}
	else if (lRes == ERROR_FILE_NOT_FOUND)
	{

	}

	return result;
}

static std::string GetFireFoxInstallationPath()
{
	std::string result;
	result = GetFireFoxInstallationPathForPlatform(eX64);
	if (result == "")
	{
		result = GetFireFoxInstallationPathForPlatform(eX32);
	}
	return result;
}



FirefoxProcess::FirefoxProcess()
{


	std::string exePath = GetFireFoxInstallationPath();
    //start Firefox Process
   


	STARTUPINFO startupInfo; 

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof startupInfo; 
	BOOL bCreated = CreateProcess(exePath.c_str()," -start-debugger-server 6000", NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &m_processInfo);
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
