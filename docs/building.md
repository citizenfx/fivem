
# Building FiveM
To build FiveM's client components, you need the following dependencies:
* A Windows machine with Visual Studio 2019 (Build Tools/Community is fine) installed with the C++ and UWP workloads.
* [Boost 1.71.0](https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.7z), extracted to a path defined by the environment variable `BOOST_ROOT`.
* [Modified CEF](https://runtime.fivem.net/build/cef/cef_binary_73.0.0-cef-patchset.1936+ga086e57+chromium-73.0.3683.75_windows64_minimal.zip), extracted to `vendor/cef` in the build tree.
* [Python 2.7.x](https://python.org/) in your PATH as `python`. This is still Python 2 due to a dependency on Mozilla `xpidl`, which hasn't been ported to Python 3.
* [MSYS2](https://www.msys2.org/) at `C:\msys64\` which is where the installer places it.

Then, execute the following commands in a `cmd.exe` shell to set up the build environment:
```dos
set BOOST_ROOT=C:\libraries\boost_1_63_0
set PATH=%path%;C:\tools\python27amd64
git clone https://github.com/citizenfx/fivem.git
cd fivem
git submodule init
git submodule update --recursive
prebuild
cd code
:: or --game=server/--game=rdr3
.\tools\ci\premake5.exe vs2019 --game=five
```

... and now you can open `build/five/CitizenMP.sln` in Visual Studio, or compile it from the command line with MSBuild.

## Setting up testing enviroment
#### Clientside
* To run modified client you need to run it in **dev mode**. To do this, create a new file in FiveM appdata folder (where FiveM.exe is located) and name it `FiveM.exe.formaldev`. If a file like this exists, launching FiveM will ensure dev mode and will skip updater *(that would otherwise overwrite your modified files with downloaded ones)*
* In order to launch modified client *(without having it crash a few seconds after launch)* you need to disable adhesive.dll. To do this, open file named *components.json* and remove line containing `"adhesive",`.
#### Serverside
* To set up a server that will let you connect with a modified client, you need to disable serversided *svadhesive.dll*. Open server's *components.json* and remove line containing `"svadhesive",`.
* You will also need to disable *sessionmanager* resource in your server.cfg as well as set the server to run in `sv_lan 1`.

## Additional information
* After compiling the project, it is necessary to copy/obtain additional files and folders to prepare the client. You can use/view [build.ps1](https://github.com/citizenfx/fivem/blob/master/code/tools/ci/build.ps1 "build.ps1") to see what is necessary for client to function properly and where to copy it from. You can also just download regular client and swap desired DLLs for the ones you changed.
* For testing natives/custom functions you can use resource called `runcode`, it is one of the default resources that comes with the server. Start it in your server.cfg and enter `127.0.0.1:30120/runcode/` in your browser. You can then run lua code dynamically without having to change/restart resources manually. 
