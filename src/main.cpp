#include "FirefoxDriver.h"
#include "args.hxx"
#include <iostream>

int main(int argc, char** argv)
{

	//parse command line args
	args::ArgumentParser parser("FireFox Driver CLI");
	args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
	args::Group flagGroup(parser);
	args::Flag listTab(flagGroup, "listTabs", "List all opened tab within Firefox", { 'l',"listTabs" });
	args::ValueFlag<string> newTab(flagGroup, "newTab", "Open new tab within Firefox", { 'n',"new" });
	args::ValueFlag<int> closeTab(flagGroup, "closeTab", "Close Tab id", { 'c',"close" });
	args::Flag	navigateTab(flagGroup, "nabivateTo", "Navigate tab to a an URL", { "navigateTo" });
	args::ValueFlag<int> tabId(flagGroup, "ID", "tab ID", { "id" });
	args::ValueFlag<string> navigateUrl(flagGroup, "url", "Url", { "url" });
	args::ValueFlag<int>  reloadTab(flagGroup, "reload", "Reload Tab", { "reload" });
	
	



	try
	{
		parser.ParseCLI(argc, argv);
	}
	catch (args::Help)
	{
		std::cout << parser;
		return 0;
	}
	catch (args::ParseError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	catch (...)
	{
		; //TBD
	}
	
	
	FireFoxDriver ffDriver;
	if (listTab)
	{
		auto tabList = ffDriver.GetTabList();
		int i=1;
		for (auto& it : tabList)
		{
			
			cout << "Tab #" << i << ": " << endl;
			cout << "	" << "title: " << it.GetTitle() << endl;
			cout << "	" << "URL: " << it.GetURL() << endl;
			cout << "	" << "actor" << it.GetActor() << endl;
			++i;
		}
	}
	if (newTab)
	{
		ffDriver.OpenNewTab(args::get(newTab));
	}
	if (closeTab)
	{
		int tabToCloseId = args::get(closeTab);
		auto allTabs = ffDriver.GetTabList();
		if (allTabs.size() >= tabToCloseId && tabToCloseId >0) {
			ffDriver.CloseTab(allTabs[tabToCloseId - 1]);
		}
	}
	if (navigateTab)
	{
		if (tabId && navigateUrl)
		{
			auto allTabs = ffDriver.GetTabList();
			int tabIDvalue = args::get(tabId);
			if (allTabs.size() >= tabIDvalue && tabIDvalue >0) {
				
				string url = args::get(navigateUrl);
				ffDriver.NavigateTo(allTabs[tabIDvalue - 1], url);
			}

		}
		else if (!tabId)
		{
			cerr << "tab id forgotten" << endl;
		}
		else if (!navigateUrl)
		{
			cerr << "url forgotten" << endl;
		}
	}

	if (reloadTab)
	{
		auto allTabs = ffDriver.GetTabList();
		int TabId = args::get(reloadTab);
		if (allTabs.size() >= TabId && TabId >0) {

			ffDriver.ReloadTab(allTabs[TabId-1]);
		}
	}
	
    return 0;
}