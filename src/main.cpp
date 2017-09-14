#include "FirefoxDriver.h"

int main(int argc, char** argv)
{
    FireFoxDriver ffDriver;
	auto tabList = ffDriver.GetTabList();
	ffDriver.NavigateTo(tabList[0], "http://facebook.com");
    return 0;
}