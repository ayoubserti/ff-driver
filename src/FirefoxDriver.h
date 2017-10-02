/*
    {{license}}
*/

#ifndef __FIREFOXDRIVER_H__
#define __FIREFOXDRIVER_H__

#include "config.h"

#include <functional>
#include <string>
#include <vector>


//forward declation
class Tab_Impl;
class Request;
class FireFoxDriver_Impl;
class JSONPacket;



class PUBLIC_API Tab {

	
    public:
	
	Tab();

	
    virtual const std::string& GetURL() const ;
    virtual const std::string& GetTitle() const;
	virtual const std::string& GetActor() const;
	virtual const std::string& GetConsoleActor() const;

	virtual Tab* Clone() const ;

	virtual ~Tab();
};




class PUBLIC_API FireFoxDriver  {
	

public:
	typedef std::function<void(const JSONPacket&)>&&  CallBackType;

	static FireFoxDriver* Create(const std::string& optArgs = "");
    
    
    /*
        @function GetTabList
        @info     return vector of opened tab
		@params	
					-inCB callback function 
        @return   vector of Tab object
    */
    

	virtual void GetTabList(std::function<void(const std::vector<Tab*>&)>&&  inCB);

	virtual void GetTabList(std::function<void(const std::vector<Tab>&)>&&  inCB);
    
    /*
        @function OpenNewTab
        @info    open new tab in the current firefox instance
		@params 
				-url to navigate to
				-inCB callback function
        @warning   
    */

	virtual void     OpenNewTab(const std::string& url, CallBackType inCB);


    /*
        @function NavigateTo
    */
    
	virtual void	NavigateTo(const Tab& inTab, const std::string& inUrl, CallBackType  inCB);
	

    /*
        @function   CloseTab
    */
	virtual void CloseTab(const Tab& inTab, CallBackType inCB);

    /*
     @function      ReloadTab
    */

	virtual  void ReloadTab(const Tab& inTab, CallBackType inCB);

	/*
	@function EvaluteJS
	@params  
			-inTab tab where the javascript code will be evaluated
			-inScript  script to evaluate
			-inCB	   callback function

	@return 
			return result as text
	*/

	virtual void EvaluateJS(const Tab& inTab, const std::string& inScript, CallBackType inCB);

	/*
		@function  AttachTab
		@info	   attach to Tab and subscribe to events; 
				   this function is asynchronous
		@params
				-inTab tab to attach
				-inCB function to execute for every event comming
		
		@return void

	*/

	virtual void	AttachTab(const Tab& inTab, CallBackType inCB);

	
    
	/*
		@function	Run
		@info		run underlaying io_service for aynchronous operation
					it should be the last line of code because it will block
	*/
	virtual void Run();



	/*
		@function OnConnect
		@info		install the callback function to be executed when The driver is connected to FF
	*/
	virtual void OnConnect(std::function<void(void)>&&);


	/*
		@function	Stop
		@info		stop event polling, the main program continue 
		@warning	doesn't close socket with FF 
	*/

	virtual void Stop();
	

};


#endif //__FIREFOXDRIVER_H__