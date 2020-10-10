# Building Cfx.re

## Generate solution

To build FiveM, RedM or FXServer on Windows you need the following dependencies:

* A Windows machine with Visual Studio 2019 (Build Tools/Community is fine) installed with the following workloads:
  - .NET desktop environment
  - Desktop development with C++
  - Universal Windows Platform development
  
  You can install these workloads by going to "Tools" -> "Get Tools and Features..." -> Check the checkboxes -> Click "Modify" in the bottom right corner.
  
* [Boost 1.71.0](https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.7z), extracted to a path defined by the environment variable `BOOST_ROOT`.
* [Modified CEF](https://runtime.fivem.net/build/cef/cef_binary_83.0.0-shared-textures.2175+g5430a8e+chromium-83.0.4103.0_windows64_hf1_minimal.zip), extracted to `vendor/cef` in the build tree.
* [Python 2.7.x](https://python.org/) in your PATH as `python`. This is still Python 2 due to a dependency on Mozilla `xpidl`, which hasn't been ported to Python 3.
* [MSYS2](https://www.msys2.org/) at `C:\msys64\` which is where the installer places it.
* [Node.js](https://nodejs.org/en/download/) and [Yarn](https://classic.yarnpkg.com/en/docs/install/) in your PATH as `yarn`.

Then, execute the following commands in a `cmd.exe` shell to set up the build environment:

```bat
set BOOST_ROOT=C:\libraries\boost_1_63_0
set PATH=%path%;C:\tools\python27amd64
git clone https://github.com/citizenfx/fivem.git
cd fivem
git submodule init
git submodule update --recursive
cd code

:: or --game=server/--game=rdr3
.\tools\ci\premake5.exe vs2019 --game=five
```

... and now you can open `build/five/CitizenMP.sln` in Visual Studio, or compile it from the command line with MSBuild.

### Set up data files for `five`

After building the FiveM client, you should be having files such as `/code/bin/five/debug/v8.dll` exist automatically. Manual copying is no longer required.

**Symlink `cache` directory** (optional)

The `/code/bin/five/debug/cache` directory can get quite large and is equivalent to the `%LocalAppData%/FiveM/FiveM.app/cache` directory, so you should use a **symlink** to save disk space.

If you don't know how to do that, here's how:

1. Navigate to `/code/bin/five/debug`.
2. Delete the `cache` folder if it already exists.
3. Hold <kbd>Shift</kbd>, right click empty space in `/code/bin/five/debug`, and select "Open PowerShell window here".
4. Type this: `New-Item -ItemType SymbolicLink -Path "cache" -Target "$env:localappdata/FiveM/FiveM.app/cache"`.
5. You should now see a `cache` folder inside `/code/bin/five/debug`.

**Known issues**

- Game starts but all I see is the main menu background, my cursor and (when I press F8) a console!
  
  This directory is probably missing: `/code/bin/five/debug/citizen/ui`.
  
  UI building might have failed, but you can copy this directory `%LocalAppData%/FiveM/FiveM.app/citizen/ui` from your main installation of FiveM.
- Windows error code 126, or some issues with other DLLs.

  It's possible that you accidentally missed a step or skipped a file that should have been copied. Start this section over from scratch; do not pass go, do not collect £200.

- Windows error code 127.

  Add an antivirus exclusion for the repository, try rebuilding, and see if it works this time.
