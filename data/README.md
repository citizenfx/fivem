# CitizenFX Runtime Data

This directory contains data which will be copied to the project layout directory on build, and is required at runtime.

## data/
* [**client/**](./client/): Shared data for all game clients, and full data for FiveM.
* [**client_rdr/**](./client_rdr/): Specific runtime data for RedM.
* [**launcher/**](./launcher/): Specific runtime data for the Cfx.re Compositing Launcher.
* [**server/**](./server/): Shared runtime data for FXServer.
* [**server_linux/**](./server_linux/): Shared runtime data for FXServer on Linux.
* [**server_proot/**](./server_proot/): Runtime data for the Alpine root build of FXServer on Linux.
* [**server_windows/**](./server_windows/): Runtime data for FXServer on Windows.
* [**shared/**](./shared/): Shared runtime data for all builds. Also includes canonical ScRT high-level source code for
  Lua/JS runtimes.