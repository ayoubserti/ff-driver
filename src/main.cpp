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
			++i;
		}
	}
	if (newTab)
	{
		ffDriver.OpenNewTab();
	}
	
    return 0;
}