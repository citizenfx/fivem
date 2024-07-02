---
ns: CFX
apiset: client
game: rdr3
---
## LOAD_TRACKS_FROM_FILE

```c
BOOL LOAD_TRACKS_FROM_FILE(char* resourceName, char* fileName);
```

Define the XML file in the resource's fxmanifest under the `file(s)` section, along with any extra track `*.dat` files.
WARNING: Running this native while any trains are spawned WILL crash the client! Make sure the resource using this native is one of the first resources to load.

## Examples

```lua
local success = LoadTracksFromFile('my-resource-name', 'traintracks.xml')
```

## Parameters
* **resourceName**: The name of the resource containing your modified tracks
* **fileName**: The name of the XML file

## Return value
True on success.