---
ns: CFX
apiset: server
---
## GET_BUILD_NUMBER

```c
int GET_BUILD_NUMBER();
```

**Note:** This build number is not the same as the client `gamebuild`. This build refers to the artifact version of `FXServer`

## Return value
Returns the build number of FXServer.

## Examples
```lua
local serverVersion = GetBuildNumber()
if serverVersion < 10309 then
    print("Your FXServer is outdated, please update.")
end
```

```js
const serverVersion = GetBuildNumber();
if (serverVersion < 10309) {
    console.log("Your FXServer is outdated, please update.");
}
```

```cs
using static CitizenFX.Core.Native.API;

int serverVersion = GetBuildNumber();
if (serverVersion < 10309) {
    Debug.WriteLine("Your FXServer is outdated, please update.");
}
```