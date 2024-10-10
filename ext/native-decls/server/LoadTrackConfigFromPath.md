---
ns: CFX
apiset: server
game: gta5
---
## LOAD_TRACK_CONFIG_FROM_PATH

```c
BOOL LOAD_TRACK_CONFIG_FROM_PATH(char* resourceName, char* fileName);
```

Loads custom train track config information for the server to use alongside [`CREATE_TRAIN`](#_0x7AC8C6B9)

## Examples

```lua
LoadTrackConfigFromPath('trains', 'traintracks.xml')
```

## Parameters
* **rseourceName**: The name of the resource containing the track config
* **fileName**: The name of the file

