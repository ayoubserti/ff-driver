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
      --navigateTo                      Navigate tab to a an URL
      --id=[ID]                         tab ID
      --url=[url]                       Url

```

## Build

ff-driver depends on 2 external libraries 
- `asio` for socket and networking stuff
- `rapidjson` to deal with JSON structure; parsing, serialization and attribute look up

`CMake` is used as project generator

Retrieve source and dependenices:

```
$ git clone http://gitlab.siriomedia.com/app-factory/ff-driver.git
$ cd ff-driver
$ git submodule update --init

```

Generate VS project

```
$ mkdir build
$ cd build 
$ cmake ../

```
The project will be generated in `build` folder.

Now Open VS solution and build


