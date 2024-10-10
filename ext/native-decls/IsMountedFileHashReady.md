---
ns: CFX
apiset: client
---
## IS_MOUNTED_FILE_HASH_READY

```c
BOOL IS_MOUNTED_FILE_HASH_READY(char* path);
```

This native is used to check if the hash of a file is ready for use with [`GET_MOUNTED_FILE_HASH`](#_0xC1657E48_).

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

Returns `true` if the hash is ready, `false` otherwise.