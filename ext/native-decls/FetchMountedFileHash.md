---
ns: CFX
apiset: client
---
## FETCH_MOUNTED_FILE_HASH

```c
void FETCH_MOUNTED_FILE_HASH(char* path);
```

This native will fetch the file and calculate its hash for use with [`GET_MOUNTED_FILE_HASH`](#_0xC1657E48).
Use [`IS_MOUNTED_FILE_HASH_READY`](#_0xEEBC88EC) to check if the hash is ready.

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
