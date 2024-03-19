---
ns: CFX
apiset: client
game: gta5
---
## CALL_MINIMAP_SCALEFORM_FUNCTION

```c
BOOL CALL_MINIMAP_SCALEFORM_FUNCTION(int miniMap, char* fnName);
```

This is similar to the PushScaleformMovieFunction natives, except it calls in the `TIMELINE` of a minimap overlay.

## Parameters
* **miniMap**: The minimap overlay ID.
* **fnName**: A function in the overlay's TIMELINE.

## Return value
