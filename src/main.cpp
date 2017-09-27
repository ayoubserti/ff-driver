#define RAPIDJSON_HAS_STDSTRING 1
#include "FirefoxDriver.h"
#if _WIN32
#undef GetObject
#endif
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
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
	args::ValueFlag<int>   attach(flagGroup, "Attach to Tab", "subscribe to tab event", { "attach" });

	args::Group optionGroup(parser);

	args::ValueFlag<string> optArgs(optionGroup, "optional argument to be passed to FF", "optional argument to be passed to FF", { "option" });

	args::Flag	 demo(flagGroup, "demo", "demo", { "demo" });


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
	



	if (demo)
	{
		//demo option
		FireFoxDriver ffDriver("-url http://wikipedia.com");
		Tab tab;
		ffDriver.OnConnect([&]() {
			
			ffDriver.GetTabList([&](const vector<Tab>& tabList) {
				
				tab = *tabList.rbegin();
				ffDriver.ReloadTab(tab,  [&](const JSONPacket&) {  //tricky action to catch tab navigated events
					ffDriver.AttachTab(tab, [&](const JSONPacket& packet) {

						rapidjson::Document document;
						if (document.Parse(packet.GetMsg()).HasParseError())
						{
							std::cerr << "Error while Parsing recieved JSON:  " << document.GetParseError() << std::endl;
							ffDriver.Stop();
							return;
						}
						else
						{
							auto obj = document.GetObject();
							if (obj.HasMember("type") && obj.HasMember("state"))
							{
								if ((string(obj["type"].GetString()) == "tabNavigated") && (string(obj["state"].GetString()) == "stop"))
								{
									ffDriver.EvaluateJS(tab, "alert('hello')", [&](const JSONPacket&) {
										ffDriver.Stop();
									});
								}
							}
						}

					});
				});
				
			});
		});

		ffDriver.Run();
		return 0;
	}



	string options = args::get(optArgs);
	FireFoxDriver ffDriver(options);


	

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
	else if (newTab)
	{
		ffDriver.OnConnect([&]() {
			ffDriver.OpenNewTab(args::get(newTab), [&](const JSONPacket&) { ffDriver.Stop(); });

		});
		ffDriver.Run();
	}
	else if (closeTab)
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
	else if (navigateTab)
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

	else if (reloadTab)
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
	
	else if (evaluateJS)
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
	else if (attach)
	{
		int tabId = args::get(attach);
		ffDriver.OnConnect([&]() {
			ffDriver.GetTabList([&](const vector<Tab>& allTabs) {
				if (allTabs.size() >= tabId && tabId > 0) {
					
					ffDriver.AttachTab(allTabs[tabId - 1], [&](const JSONPacket& p) {
						cout << p.GetMsg() << endl;
					});
				}
			});
		});

		ffDriver.Run();
	}

    return 0;
}

