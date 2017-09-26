#include "FirefoxDriver.h"
#include "args.hxx"
#include <iostream>
#include <fstream>


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
		
		ffDriver.OnConnect([&]() {
			ffDriver.GetTabList([&](const vector<Tab>& tabList) {
				int i = 1;
				for (auto& it : tabList)
				{

					cout << "Tab #" << i << ": " << endl;
					cout << "	" << "title: " << it.GetTitle() << endl;
					cout << "	" << "URL: " << it.GetURL() << endl;
					cout << "	" << "actor" << it.GetActor() << endl;
					++i;
				}
				ffDriver.Stop();
			});
		});

		ffDriver.Run();

		
	}
	if (newTab)
	{
		ffDriver.OnConnect([&]() {
			ffDriver.OpenNewTab(args::get(newTab), [&](const JSONPacket&) { ffDriver.Stop(); });

		});
		ffDriver.Run();
	}
	if (closeTab)
	{
		int tabToCloseId = args::get(closeTab);
		ffDriver.OnConnect([&]() {
			ffDriver.GetTabList([&ffDriver, tabToCloseId](const vector<Tab>& allTabs) {
				if (allTabs.size() >= tabToCloseId && tabToCloseId > 0) {
					ffDriver.CloseTab(allTabs[tabToCloseId - 1], [&](const JSONPacket&) {
						ffDriver.Stop();
					});
				}
			});
		});

		ffDriver.Run();
		
	}
	if (navigateTab)
	{
		ffDriver.OnConnect([&]() {
			if (tabId && navigateUrl)
			{
				int tabIDvalue = args::get(tabId);
				string url = args::get(navigateUrl);
				ffDriver.GetTabList([&ffDriver, tabIDvalue, url](const vector<Tab>& allTabs) {
					if (allTabs.size() >= tabIDvalue && tabIDvalue > 0) {

						ffDriver.NavigateTo(allTabs[tabIDvalue - 1], url, [&](const JSONPacket&) {
							ffDriver.Stop();
						});
					}
				});


			}
			else if (!tabId)
			{
				cerr << "tab id forgotten" << endl;
			}
			else if (!navigateUrl)
			{
				cerr << "url forgotten" << endl;
			}
		});

		ffDriver.Run();
	}

	if (reloadTab)
	{
		ffDriver.OnConnect([&]() {
			int TabId = args::get(reloadTab);

			ffDriver.GetTabList([&ffDriver, TabId](const vector<Tab>& allTabs) {
				if (allTabs.size() >= TabId && TabId > 0) {
					ffDriver.ReloadTab(allTabs[TabId - 1], [&](const JSONPacket&) {
						ffDriver.Stop();
					});
				}
			});
		});

		ffDriver.Run();
		
	}
	
	if (evaluateJS)
	{
		ffDriver.OnConnect([&]() {
			int TabId = args::get(tabId);

			ffDriver.GetTabList([&](const vector<Tab>& allTabs) {
				string jscode = "";
				if (file)
				{
					std::fstream fileScript;
					fileScript.open(args::get(file), std::ios_base::in);
					if (!fileScript.is_open()) {
						cerr << "Can't open file " << args::get(file) << endl;
						ffDriver.Stop();
						return;
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
					ffDriver.Stop();
					return;
				}

				if (allTabs.size() >= TabId && TabId >0) {

					ffDriver.EvaluateJS(allTabs[TabId - 1], jscode, [&](const JSONPacket& p) {
						cout << p.GetMsg() << endl;
						ffDriver.Stop();
					});
				}
				
			});
		});

		ffDriver.Run();
		
	}

    return 0;
}

