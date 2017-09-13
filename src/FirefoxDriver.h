/*
    {{license}}
*/

#ifndef __FIREFOXDRIVER_H__
#define __FIREFOXDRIVER_H__

#include <string>
#include <vector>

#include "ProcessLauncher.h"

using namespace std;

class Tab{

   

    string m_TabURL;
    string m_title;
    string m_tabActor;

    public:

    string GetURL();
    string GetTitle();
    string GetActor();
};


class FireFoxDriver : public FirefoxProcess {

    
    FireFoxDriver(const FireFoxDriver& ) = delete;
    FireFoxDriver( FireFoxDriver&& ) = delete;
    
    
    
    public:
    
    /*
        @ctor
        @info Create new Firefox process
    */
    FireFoxDriver();
    
    /*
        @function GetTabList
        @info     return vector of opened tab
        @return   vector of Tab object
    */
    std::vector<Tab> GetTabList();
    
    /*
        @function OpenNewTab
        @info    open new tab in the current firefox instance
        @warning    not implemented
    */

    Tab     OpenNewTab();


    /*
        @function NavigateTo
    */
    void    NavigateTo(const Tab& inTab, std::string inUrl);
    

    /*
        @function   CloseTab
    */
    void CloseTab(const Tab& inTab);
    
};


#endif //__FIREFOXDRIVER_H__