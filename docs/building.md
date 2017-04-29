# Building FiveM
To build FiveM's client components, you need the following dependencies:
* A Windows machine with Visual Studio 2017 (Build Tools/Community is fine) installed.
* [Boost 1.63.0](https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.7z/download), extracted to a path defined by the environment variable `BOOST_ROOT`.
* [Modified CEF](https://runtime.fivem.net/build/cef/cef_binary_3.3071.1610.g5a5b538_windows64_minimal.zip), extracted to `vendor/cef` in the build tree.
* [Python 2.7](https://python.org/) in your PATH as `python`.
* [Premake 5.0](https://premake.github.io/download.html) somewhere it can be found.

Then, execute the following commands in a `cmd.exe` shell to set up the build environment:
```dos
set BOOST_ROOT=C:\libraries\boost_1_63_0
set PATH=%path%;C:\tools\python27amd64
git clone https://github.com/citizenfx/fivem.git
git submodule init
git submodule update --recursive
prebuild
cd code
premake5 vs2017 --game=five
```

... and now you can open `build/five/CitizenMP.sln` in Visual Studio, or compile it from the command line with MSBuild.