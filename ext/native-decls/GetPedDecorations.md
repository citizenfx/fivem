---
ns: CFX
apiset: shared
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

On the client, this command will return undefined data if invoked on a remote player ped.

On the server, this only works on players and not on other peds, and requires `game_enablePlayerDecorationSyncFix` to be enabled. An empty list is returned otherwise.

## Parameters
* **ped**: The ped you want to retrieve data for.

## Return value
An object containing a list of applied decorations.
