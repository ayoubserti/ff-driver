#include "FirefoxDriver.h"

int main(int argc, char** argv)
{
    FireFoxDriver ffDriver;
	auto tabList = ffDriver.GetTabList();
    return 0;
}