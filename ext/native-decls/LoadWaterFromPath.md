---
ns: CFX
apiset: client
game: gta5
---
## LOAD_WATER_FROM_PATH

```c
BOOL LOAD_WATER_FROM_PATH(char* resourceName, char* fileName);
```

Define the xml in a resources fxmanifest, under the file(s) section.

## Examples

```lua
local success = LoadWaterFromPath('my-resource-name', 'water-all-over-the-place.xml')
```

## Parameters
* **resourceName**: The name of the resource containing your modified water definition
* **fileName**: The name of the file

## Return value
Returns true on success.