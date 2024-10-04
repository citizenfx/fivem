---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_DECORATIONS

```c
object GET_PED_DECORATIONS(Ped ped);
```

Returns a list of decorations applied to a ped.

The data returned adheres to the following layout:
```
[ [ collectionHash1, overlayHash1 ], ..., [c ollectionHashN, overlayHashN ] ]
```

This command will return undefined data if invoked on a remote player ped.

## Parameters
* **ped**: The ped you want to retrieve data for.

## Return value
An object containing a list of applied decorations.
