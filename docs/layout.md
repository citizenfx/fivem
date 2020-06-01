# Source code layout for Cfx.re/CitizenFX

The CitizenFX codebase is a vast set of multiple components, many of which having nested README.md files explaining
their purpose further in the codebase.

This document is a general guideline towards these directories.

## Root
* [**code/**](../code/): Core native project code, and the root of the native build.
* [**data/**](../data/): Runtime data for the projects to use.
* [**ext/**](../ext/): Extra supporting projects, generally either standalone or otherwise only depending on files/data in `ext`/ itself.
* [**vendor/**](../vendor/): Submodules for project dependencies.