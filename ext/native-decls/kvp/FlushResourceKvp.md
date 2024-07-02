---
ns: CFX
apiset: server
---
## FLUSH_RESOURCE_KVP

```c
void FLUSH_RESOURCE_KVP();
```

Nonsynchronous operations will not wait for a disk/filesystem flush before returning from a write or delete call. They will be much faster than their synchronous counterparts (e.g., bulk operations), however, a system crash may lose the data to some recent operations.

This native ensures all `_NO_SYNC` operations are synchronized with the disk/filesystem.

## Examples
```lua
-- Bulk write many <key, value> pairs to the resource KVP.
local key = "bug_%d"
local value = "unintended_feature_%d"
for i=1,10000 do
	SetResourceKvpNoSync(key:format(i), value:format(i))
end

-- Ensure all data is synchronized to the filesystem
FlushResourceKvp()
```
