#include "FirefoxDriver.h"
#include "args.hxx"
#include <iostream>
#include <fstream>


#define _TEST 1





int main(int argc, char** argv)
{
#if _TEST

	//for testing purpose
	//TODO: cleanup when finish with branch
	FireFoxDriver ffDriver;

	Tab tabToAttach;
	
	auto completion= [&]() {

		ffDriver.GetTabList([&](const vector<Tab>& tabs) {
			int i = 1;

			cout << "root actor" << endl;

			for (auto& it : tabs)
			{

				cout << "Tab #" << i << ": " << endl;
				cout << "	" << "title: " << it.GetTitle() << endl;
				cout << "	" << "URL: " << it.GetURL() << endl;
				cout << "	" << "actor: " << it.GetActor() << endl;
				++i;
			}
			tabToAttach = tabs[1];


			ffDriver.NavigateTo(tabs[0], "http://www.google.com",[ & ](const JSONPacket& packet) {

				

				ffDriver.AttachTab(tabToAttach, [](const JSONPacket& packet) {

					cout << "attach event " << endl;

				});

			});

			
		});

		
	};
	ffDriver.OnConnect(completion);
	ffDriver.Run();
	return 0;

#else
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
	args::Flag evaluateJS(flagGroup, "evaluateJS", "Evaluate Javascript code on Tab", { "eval" });
	args::ValueFlag<string> file(flagGroup, "file to evaluate", "File to evaluate", { "file" });
	args::ValueFlag<string> text(flagGroup, "text to evaluate", "text to evaluate", { "text" });



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
	
	if (evaluateJS)
	{
		auto allTabs = ffDriver.GetTabList();
		int TabId = args::get(tabId);
		string jscode = "";
		if (file)
		{
			std::fstream fileScript;
			fileScript.open(args::get(file), std::ios_base::in);
			if (!fileScript.is_open()) {
				cerr << "Can't open file " << args::get(file) << endl;
				return 2;
			}
			std::string contentScript((std::istreambuf_iterator<char>(fileScript)),
				(std::istreambuf_iterator<char>()));
			jscode = contentScript;

		}
		else if (text)
		{
			jscode = args::get(text);
		}
		else
		{
			cerr << "You must use --file or --text with --eval command" << endl;
			return 1;
		}

		if (allTabs.size() >= TabId && TabId >0) {

			cout << ffDriver.EvaluateJS(allTabs[TabId - 1],jscode) << endl;
		}
	}
    return 0;
#endif
}

