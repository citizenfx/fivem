# Building FiveM
To build FiveM's client components, you need the following dependencies:
* A Windows machine with Visual Studio 2019 (Build Tools/Community is fine) installed.
* [Boost 1.63.0](https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.7z/download), extracted to a path defined by the environment variable `BOOST_ROOT`.
* [Modified CEF](https://runtime.fivem.net/build/cef/cef_binary_73.0.0-cef-patchset.1936+ga086e57+chromium-73.0.3683.75_windows64_minimal.zip), extracted to `vendor/cef` in the build tree.
* [Python 2.7.x](https://python.org/) in your PATH as `python`.
* [Premake 5.0](https://premake.github.io/download.html) somewhere it can be found.
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
premake5 vs2017 --game=five
```

... and now you can open `build/five/CitizenMP.sln` in Visual Studio, or compile it from the command line with MSBuild.
