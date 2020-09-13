# CitizenFX Development Kit

## Building prerequisites

 - [Node.JS v12.18.x](https://nodejs.org/), **version is important!**
 - [`Yarn package manager`](https://classic.yarnpkg.com/en/docs/install#windows-stable)


## Development flow

 - `yarn install`
 - `yarn start`
 - Run FxDK:
 ```
 .\FiveM.exe -fxdk +set sdk_url http://%PC_NAME%:3000 +set sdk_root_path "%SDK_ROOT_PATH%"
 ```
 Variables here:
  - `%PC_NAME%` - your pc name, you may obtain that running `hostname` command.
  - `%SDK_ROOT_PATH%` - path to `sdk-root` resource, for example: `/dev/fivem/ext/sdk/resources/sdk-root`,
    note that it expects `sdk-game` resource to be near `sdk-root`, as you may have seen in fivem source code.

## Building

Building is handled by `../ext/sdk-build/build.cmd`, you can read that to see how we build it.
