# Cfx.re Development Kit

## Building prerequisites

 - [Node.JS v12.18.x](https://nodejs.org/), **use of v12 is important!**
 - [`Yarn package manager`](https://classic.yarnpkg.com/en/docs/install#windows-stable)


## Development flow

 - `yarn install`
 - `yarn start`
 - Run FxDK:
 ```
 .\FiveM.exe -fxdk +set sdk_url http://%COMPUTERNAME%:3000 +set sdk_root_path "%SDK_ROOT_PATH%"
 ```
 Variables here:
  - `%SDK_ROOT_PATH%` - path to `sdk-root` resource, for example: `C:/dev/fivem/ext/sdk/resources/sdk-root`,
    note that it expects `sdk-game` resource to be near `sdk-root`, as in the original source tree.

    It also should use slashes, not backslashes.

## Building

Building is handled by `../ext/sdk-build/build.cmd`, you can read that to see how we build it.
