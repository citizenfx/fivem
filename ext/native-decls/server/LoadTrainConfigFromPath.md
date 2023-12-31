---
ns: CFX
apiset: server
game: gta5
---
## LOAD_TRAIN_CONFIG_FROM_PATH

```c
BOOL LOAD_TRAIN_CONFIG_FROM_PATH(char* resourceName, char* fileName);
```

Loads custom train config information for the server to use alongside [`CREATE_TRAIN`](#_0x7AC8C6B9)

## Examples

```lua
LoadTrainConfigFromPath('trains', 'trains.xml')
```

## Parameters
* **rseourceName**: The name of the resource containing the train config
* **fileName**: The name of the file

