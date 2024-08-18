---
ns: CFX
apiset: client
game: gta5
---

## OVERRIDE_POP_GROUPS

```c
void OVERRIDE_POP_GROUPS(char* path);
```

Replaces the `popgroups` (CPopGroupList) meta file with the file in the specified path.

## Parameters
* **path**: The file path to load (`popgroups`, `@resource/popgroups`), or `null` to reload the default population groups file.

## Examples
```lua
-- fxmanifest.lua:
file 'popgroups_dlc.xml'

-- client.lua:
OverridePopGroups('popgroups_dlc.xml')

-- restore the original after five minutes
Wait(1000 * 60 * 5)
OverridePopGroups(nil)
```