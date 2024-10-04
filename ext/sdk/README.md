# Cfx.re Development Kit

## Building prerequisites

 - [Node.JS v16.13.x](https://nodejs.org/)
 - [`Yarn package manager`](https://classic.yarnpkg.com/en/docs/install#windows-stable)


## Development flow
 - `git submodule update --init` **Must be done from the FiveM source code root**
 - `yarn install`
 - `yarn start`
 - Run FxDK:
 ```
 .\FiveM.exe -fxdk +set sdk_url http://%COMPUTERNAME%:3000 +set sdk_root_path "%SDK_ROOT_PATH%"
 ```
 Variables here:
  - `%SDK_ROOT_PATH%` - path to `sdk-root` resource, for example: `C:/dev/fivem/ext/sdk/resources/sdk-root`,
    note that it expects `sdk-game` resource to be near `sdk-root`, as in the original source tree.

    **Use of slashes (`/`) in place of `%SDK_ROOT_PATH%` is important, don't use backslashes (`\`) there.**


## Building

Building is handled by `../ext/sdk-build/build.cmd`, you can read that to see how we build it.
