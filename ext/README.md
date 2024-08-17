# CitizenFX External Projects

This directory contains external projects which are _generally_ part of the core build in some way, but not considered
part of the main project.

## ext/
* [**cfx-ui/**](./cfx-ui): The main menu interface for the game clients.
* [**event-doc-gen/**](./event-doc-gen): Generation tooling for _event_ documentation.
* [**native-decls/**](./native-decls): Native function declarations for `CFX/` natives defined in the main project.
* [**native-doc-gen/**](./native-doc-gen): Scripts to generate `native-decls` using the `native-doc-tooling` submodule.
* [**native-doc-tooling/**](./native-doc-tooling): A submodule for the latest version of `native-doc-tooling`, which is used to generate native definitions from the documentation in `native-decls`.
* [**natives/**](./natives): Code generation tooling to convert a native function database in 'intermediate' Lua format to runnable Lua/JS/C# and other intermediate formats.
* [**nuget/**](./nuget): Build files for the `CitizenFX.Core.Client` and `CitizenFX.Core.Server` NuGet packages.
* [**symbol-upload/**](./symbol-upload): A tool to collect Linux build symbols to upload to a .NET Core-style symbol server.
* [**system-resources/**](./system-resources): System resources bundled with FXServer by default.
* [**typings/**](./typings): Build files for the `@citizenfx/client` and `@citizenfx/server` NPM packages.
* [**ui-build/**](./ui-build): Scripts and data files for building `citizen/ui.zip` including root helpers, legacy UI scripts and a packed version of `cfx-ui`.