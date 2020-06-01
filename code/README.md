# CitizenFX Native Project

This directory contains the primary native CitizenFX project.

* [**client/**](./client): A historical misnomer, contains common code shared across projects, as well as the client launcher logic.
* [**components/**](./components): Contains most of the code as part of a 'component' system.
* [**deplibs/**](./deplibs): Legacy manually-vendored-in includes/lib files.
* [**server/**](./server): Contains the server launcher logic.
* [**shared/**](./shared): A confusing cognate to `client/shared/`, contains lower-level shared code.
* [**tests/**](./tests): Unused since 2014, a prototype of unit/integration tests for the framework.
* [**tools/**](./tools): Tooling used for building the native CitizenFX project.
* [**vendor/**](./vendor): Premake vendor definition files and non-submoduled dependent includes/source files.