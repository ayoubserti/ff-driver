# Firefox Driver
ff-driver is a Firefox driver library written in C++. It allow debugging firefox instance.

## API
 - `FireFoxDriver::FireFoxDriver` : constructor, launch Firefox process and connect to debugger server
 - `FireFoxDriver::GetTabList`    : method, return all Tabs within Firefox process
 - `FireFoxDriver::OpenNewtab`    : method, open new Tab and navigate to url in parameter
 - `FireFoxDriver::CloseTab`      : method, close the Tab in parameter
 - `FireFoxDriver::NavigateTo`    : method, navigate to url in parameter within the tab in parameter

 ## CLI
 FireFox Driver CLI

```
ff-driver-cmd.exe {OPTIONS}

  OPTIONS:

      -h, --help                        Display this help menu
      -l, --listTabs                    List all opened tab within Firefox
      -n[newTab], --new=[newTab]        Open new tab within Firefox
      -c[closeTab], --close=[closeTab]  Close Tab id
```
