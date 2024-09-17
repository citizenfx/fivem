# `components/`

CitizenFX implementation components.

## List

### Common
* [**citizen-game-ipc/**](./citizen-game-ipc): IPC wrapper for CfxGL.
* [**citizen-game-main/**](./citizen-game-main): Main entry point for CfxGL.
* [**citizen-legacy-net-resources/**](./citizen-legacy-net-resources): Binds `net` to the `citizen:resources:client` resource system.
* [**citizen-resources-client/**](./citizen-resources-client): Resources system, client-specific functionality.
* [**citizen-resources-core/**](./citizen-resources-core): Resources system, core functionality.
* [**citizen-resources-gta/**](./citizen-resources-gta): Resource system, GTA-specific functionality.
* [**citizen-resources-metadata-lua/**](./citizen-resources-metadata-lua): Loader for `fxmanifest.lua` and `__resource.lua` files.
* [**citizen-scripting-core/**](./citizen-scripting-core): Core scripting runtime (ScRT) functionality.
* [**citizen-scripting-lua/**](./citizen-scripting-lua): Lua ScRT.
* [**citizen-scripting-mono/**](./citizen-scripting-mono): Mono ScRT loader. Most of the implementation is in `CitizenFX.Core.dll`.
* [**citizen-scripting-v8/**](./citizen-scripting-v8): V8 ScRT.
* [**conhost-posh/**](./conhost-posh): Deprecated PowerShell console host.
* [**conhost-v2/**](./conhost-v2): dear ImGui-based console host.
* [**debug-net/**](./debug-net): Debug visualizations for `net`.
* [**debug-script/**](./debug-script): Script debugging functionality.
* [**devcon/**](./devcon): VConsole2 server.
* [**discord/**](./discord): Discord integration.
* [**font-renderer/**](./font-renderer): Watermark renderer and DirectWrite font client.
* [**glue/**](./glue): High-level 'glue' to link other components together.
* [**http-client/**](./http-client): Async cURL wrapper library.
* [**lovely-script/**](./lovely-script): Background script.
* [**n19ui/**](./n19ui): Jupiter-based UI. Not currently used.
* [**net/**](./net): Network client for game servers.
* [**net-base/**](./net-base): Base networking library for client/server projects.
* [**net-http-server/**](./net-http-server): HTTP server for `net:tcp-server`.
* [**net-tcp-server/**](./net-tcp-server): Generic transport-agnostic TCP server framework.
* [**nui-core/**](./nui-core): NUI core library.
* [**nui-gsclient/**](./nui-gsclient): NUI game server client, for the server list.
* [**nui-profiles/**](./nui-profiles): NUI user profile functionality.
* [**nui-resources/**](./nui-resources): NUI bindings to the resource system.
* [**profiles/**](./profiles): User profile functionality.
* [**pool-sizes-state/**](./pool-sizes-state): Track and validate server requests to increase sizes of pools.
* [**rage-formats-x/**](./rage-formats-x): RAGE file format library.
* [**scrbind-base/**](./scrbind-base): High-level C++ class script binding support.
* [**scrbind-formats/**](./scrbind-formats): Uses scrBind to bind to rage:formats:x.
* [**scripthookv/**](./scripthookv): ScriptHookV compatibility library.
* [**scripting-gta/**](./scripting-gta): Scripting implementation wrapper to `rage:scripting`.
* [**steam/**](./steam): Steam integration.
* [**sticky/**](./sticky): Surrogate for private `adhesive` component.
* [**template/**](./template): Template for use with new components.
* [**tool-formats/**](./tool-formats): Tool component for `rage:formats:x`.
* [**tool-vehrec/**](./tool-vehrec): Tool component to generate .#vr files.
* [**vfs-core/**](./vfs-core): Virtual File System, core library.
* [**vfs-impl-rage/**](./vfs-impl-rage): Virtual File System, RAGE implementation.
* [**voip-mumble/**](./voip-mumble): Mumble client.

### Server
* [**citizen-server-impl/**](./citizen-server-impl): Server core implementation. This is a single component due to ABI limitations
  on Linux.
* [**citizen-server-instance/**](./citizen-server-instance): Server `instance` wrapper. Factored out for dependencies.
* [**citizen-server-main/**](./citizen-server-main): Server entry point.
* [**citizen-server-monitor/**](./citizen-server-monitor): Server entry point for txAdmin monitor mode.
* [**citizen-server-net/**](./citizen-server-net): Server `net` wrapper. Factored out for dependencies.
* [**citizen-ssh-server/**](./citizen-ssh-server): ...
* [**conhost-server/**](./conhost-server): Dummy `conhost` implementation for the server.
* [**scripting-server/**](./scripting-server): Standalone scripting implementation for use in the server.
* [**vfs-impl-server/**](./vfs-impl-server): Standalone VFS implementation for use in the server.
* [**voip-server-mumble/**](./voip-server-mumble): Mumble server.

### GTA5
* [**asi-five/**](./asi-five): ASI loader.
* [**citizen-level-loader-five/**](./citizen-level-loader-five): SP level loading support.
* [**citizen-mod-loader-five/**](./citizen-mod-loader-five): .oiv mod loading support.
* [**citizen-playernames-five/**](./citizen-playernames-five): Player name overrides.
* [**devtools-five/**](./devtools-five): Developer tools.
* [**extra-natives-five/**](./extra-natives-five): High-level game-specific natives.
* [**gta-core-five/**](./gta-core-five): Low-level GTA project wrappers.
* [**gta-game-five/**](./gta-game-five): High-level GTA project wrappers.
* [**gta-mission-cleanup-five/**](./gta-mission-cleanup-five): Bindings to GTA mission cleanup.
* [**gta-net-five/**](./gta-net-five): Network functionality for GTA.
* [**gta-streaming-five/**](./gta-streaming-five): Streaming functionality for GTA.
* [**handling-loader-five/**](./handling-loader-five): Handling loader.
* [**loading-screens-five/**](./loading-screens-five): Loading screen override functionality.
* [**rage-allocator-five/**](./rage-allocator-five): Wraps RAGE allocators.
* [**rage-device-five/**](./rage-device-five): Wraps RAGE VFS.
* [**rage-graphics-five/**](./rage-graphics-five): Wraps RAGE grcore.
* [**rage-input-five/**](./rage-input-five): Wraps RAGE grcore input.
* [**rage-nutsnbolts-five/**](./rage-nutsnbolts-five): Generic 'nuts and bolts' for RAGE.
* [**rage-scripting-five/**](./rage-scripting-five): Wraps RAGE scripting.
* [**ros-patches-five/**](./ros-patches-five): ROS compatibility.

### RDR2
* [**citizen-level-loader-rdr3/**](./citizen-level-loader-rdr3): SP level loading support.
* [**citizen-playernames-rdr3/**](./citizen-playernames-rdr3): Player name overrides.
* [**extra-natives-rdr3/**](./extra-natives-rdr3): High-level game-specific natives.
* [**gta-core-rdr3/**](./gta-core-rdr3): Low-level GTA project wrappers. (RDR3 is built atop GTA/RAGE)
* [**gta-game-rdr3/**](./gta-game-rdr3): High-level GTA project wrappers.
* [**gta-mission-cleanup-rdr3/**](./gta-mission-cleanup-rdr3): ...
* [**gta-net-rdr3/**](./gta-net-rdr3): ...
* [**gta-streaming-rdr3/**](./gta-streaming-rdr3): ...
* [**rage-allocator-rdr3/**](./rage-allocator-rdr3): ...
* [**rage-device-rdr3/**](./rage-device-rdr3): ...
* [**rage-graphics-rdr3/**](./rage-graphics-rdr3): ...
* [**rage-input-rdr3/**](./rage-input-rdr3): ...
* [**rage-nutsnbolts-rdr3/**](./rage-nutsnbolts-rdr3): ...
* [**rage-scripting-rdr3/**](./rage-scripting-rdr3): ...
* [**ros-patches-rdr3/**](./ros-patches-rdr3): Symlinked ROS compatibility.