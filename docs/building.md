# Building FiveM

## Generate solution

To build FiveM or RedM you need the following dependencies:

* A Windows machine with Visual Studio 2019 (Build Tools/Community is fine) installed with the following workloads:
  - .NET desktop environment
  - Desktop development with C++
  - Universal Windows Platform development
  
  You can install these workloads by going to "Tools" -> "Get Tools and Features..." -> Check the checkboxes -> Click "Modify" in the bottom right corner.
  
* [Boost 1.71.0](https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.7z), extracted to a path defined by the environment variable `BOOST_ROOT`.
* [Modified CEF](https://runtime.fivem.net/build/cef/cef_binary_73.0.0-cef-patchset.1936+ga086e57+chromium-73.0.3683.75_windows64_minimal.zip), extracted to `vendor/cef` in the build tree.
* [Python 2.7.x](https://python.org/) in your PATH as `python`. This is still Python 2 due to a dependency on Mozilla `xpidl`, which hasn't been ported to Python 3.
* [MSYS2](https://www.msys2.org/) at `C:\msys64\` which is where the installer places it.

Then, execute the following commands in a `cmd.exe` shell to set up the build environment:

```bat
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

### Set up data files for `five`

After building the FiveM client, you should create the following files in the `/code/bin/five/debug` directory:

- `FiveM.exe.formaldev`
- `nobootstrap.txt`

If you don't do this, FiveM will auto-update using the official binaries.

**CEF**

1. Create `cef` folder inside `/code/bin/five/debug/bin`. This is our "CEF directory".
2. Copy `*.pak` files from `/vendor/cef/Resources` into our CEF directory.
3. Copy `/vendor/cef/Resources/locales/en-US.pak` into our CEF directory. (Don't create a nested `locales` directory.)
5. Copy `/vendor/cef/Resources/icudtl.dat` to `/code/bin/five/debug/bin/icudtl.dat`
6. Copy `.dll` and `.bin` that are _directly inside_ `/vendor/cef/Release` to `/code/bin/five/debug/bin`.

**Additional data files**

1. Copy contents of `/data/shared` into `/code/bin/five/debug`.
2. Copy contents of `/data/client` into `/code/bin/five/debug`.
3. **VERY IMPORTANT**: there should now be a file at `/code/bin/five/debug/components.json`. Make sure you remove the line `"adhesive"` from that file. This is the anti-cheat module and you will be banned for attempted cheating if you don't delete this line.

**Symlink `cache` directory** (optional)

The `/code/bin/five/debug/cache` directory can get quite large and is equivalent to the `%LocalAppData%/FiveM/FiveM.app/cache` directory, so you should use a **symlink** to save disk space.

If you don't know how to do that, here's how:

1. Navigate to `/code/bin/five/debug`.
2. Delete the `cache` folder if it already exists.
3. Hold <kbd>Shift</kbd>, right click empty space in `/code/bin/five/debug`, and select "Open PowerShell window here".
4. Type this: `New-Item -ItemType Junction -Path "cache" -Target "$env:localappdata/FiveM/FiveM.app/cache"`.
5. You should now see a `cache` folder inside `/code/bin/five/debug`.

**Known issues**

- Game starts but all I see is the main menu background, my cursor and (when I press F8) a console!
  
  This directory is probably missing: `/code/bin/five/debug/citizen/ui`.
  
  We still need to add instructions on how to build the UI (`/ext/ui-build`), so for now, just copy this directory `%LocalAppData%/FiveM/FiveM.app/citizen/ui` from your main installation of FiveM.
- Windows error code 126, or some issues with other DLLs.

  It's possible that you accidentally missed a step or skipped a file that should have been copied. Start this section over from scratch; do not pass go, do not collect £200.

- Windows error code 127.

  Add an antivirus exclusion for the repository, try rebuilding, and see if it works this time.
