#include "FirefoxDriver.h"

FireFoxDriver::FireFoxDriver() : FirefoxProcess()
{

}

std::vector<Tab> FireFoxDriver::GetTabList()
{
	std::vector<Tab> tabs;
	
	return tabs;
}

Tab FireFoxDriver::OpenNewTab()
{
	return Tab();
}

void FireFoxDriver::NavigateTo(const Tab & inTab, std::string inUrl)
{
}

void FireFoxDriver::CloseTab(const Tab & inTab)
{
}



