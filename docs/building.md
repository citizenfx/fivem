# Building Cfx.re

## Generate solution

To build FiveM, RedM or FXServer on Windows you need the following dependencies:

* A Windows machine with Visual Studio 2022 (Build Tools/Community is fine) installed with the following workloads:
  - .NET desktop environment
  - Desktop development with C++
  - Universal Windows Platform development
  
  You can install these workloads by going to "Tools" -> "Get Tools and Features..." -> Check the checkboxes -> Click "Modify" in the bottom right corner.
  
* [PowerShell 7](https://aka.ms/powershell-release?tag=stable) or higher.
* [Boost 1.71.0](https://boostorg.jfrog.io/artifactory/main/release/1.71.0/source/boost_1_71_0.7z), extracted to a path defined by the environment variable `BOOST_ROOT`.
* [Python 3.8 or higher](https://python.org/) with the `py` launcher installed.
* [MSYS2](https://www.msys2.org/) at `C:\msys64\` which is where the installer places it.
* [Node.js](https://nodejs.org/en/download/) and [Yarn](https://classic.yarnpkg.com/en/docs/install/) in your PATH as `node` and `yarn`.

Then, execute the following commands in a `cmd.exe` shell to set up the build environment:

```bat
set BOOST_ROOT=C:\libraries\boost_1_71_0
set PATH=C:\tools\python27amd64;%path%
git clone https://github.com/citizenfx/fivem.git -c core.symlinks=true
cd fivem
git submodule update --jobs=16 --init

:: downloads the right Chrome version for 64-bit projects
fxd get-chrome

:: or -game server/-game rdr3/-game ny
fxd gen -game five
fxd vs -game five
```

### Set up data files for `five`

After building the FiveM client, you should be having files such as `/code/bin/five/debug/v8.dll` exist automatically. Manual copying is no longer required.

**Symlink `game-storage` directory** (optional)

The `/code/bin/five/debug/data/game-storage` directory can get quite large and is equivalent to the `%LocalAppData%/FiveM/FiveM.app/data/game-storage` directory, so you should use a **symlink** to save disk space.

If you don't know how to do that, here's how:

1. Navigate to `/code/bin/five/debug`.
2. Delete the `game-storage` folder in `data` if it already exists.
3. Make a new `data` directory if it doesn't already exist.
4. Hold <kbd>Shift</kbd>, right click empty space in `/code/bin/five/debug/data`, and select "Open PowerShell window here".
5. Type this: `New-Item -ItemType SymbolicLink -Path "game-storage" -Target "$env:localappdata/FiveM/FiveM.app/data/game-storage"`.
6. You should now see a `game-storage` folder inside `/code/bin/five/debug/data`.

**Known issues**

- Game starts but all I see is the main menu background, my cursor and (when I press F8) a console!
  
  This directory is probably missing: `/code/bin/five/debug/citizen/ui`.
  
  UI building might have failed, but you can copy this directory `%LocalAppData%/FiveM/FiveM.app/citizen/ui` from your main installation of FiveM.
- Windows error code 126, or some issues with other DLLs.

  It's possible that you accidentally missed a step or skipped a file that should have been copied. Start this section over from scratch; do not pass go, do not collect £200.

- Windows error code 127.

  Add an antivirus exclusion for the repository, try rebuilding, and see if it works this time.
