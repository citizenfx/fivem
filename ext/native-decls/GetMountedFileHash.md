---
ns: CFX
apiset: client
---
## GET_MOUNTED_FILE_HASH

```c
char* GET_MOUNTED_FILE_HASH(char* path);
```

This native returns the hash of the file that is currently mounted. The hash can be useful to verify if the file has been altered. If the path is not whitelisted, the file has not been fetched or the file is not mounted, the native will return "missing".

## Examples

```lua
local hashes = {
    "platform:/levels/gta5/props/residential/v_garden/prop_airport_sale.ytd",
    "common:/data/materials/materials.dat"
} 

for _,v in pairs(hashes) do
    FetchMountedFileHash(v)

    while not IsMountedFileHashReady(v) do
        Wait(0)
    end

    print(GetMountedFileHash(v))
end
```

## Parameters
* **path**: The path to the file.

## Return value

The hash of the mounted file as a string.