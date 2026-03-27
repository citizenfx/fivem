---
ns: CFX
apiset: server
---
## GET_RESOURCE_PATH

```c
char* GET_RESOURCE_PATH(char* resourceName);
```

## Parameters
* **resourceName**: The name of the resource.

## Return value
Returns the physical on-disk path of the specified resource.

## Examples
```lua
local resource_path = GetResourcePath("monitor")

print(resource_path)
-- This would return something similar to this on Linux:
-- /mnt/ssd2/fxserver/server/alpine/opt/cfx-server/citizen//system_resources//monitor
-- F:/FxServer/server/citizen/system_resources/monitor

local resource_path = GetResourcePath("runcode")

print(resource_path)
-- This would return something similar to:
-- /mnt/ssd2/fxserver/server-data/resources//runcode
-- F:/FxServer/server-data/resources//[system]/runcode
```
