---
ns: CFX
apiset: client
---
## GET_ROPE_FLAGS

```c
int GET_ROPE_FLAGS(int rope);
```

```c
enum eRopeFlags
{
    DrawShadowEnabled = 2,
	Breakable = 4,
	RopeUnwindingFront = 8,
	RopeWinding = 32
}
```

## Parameters
* **rope**: The rope to get the flags for.

## Return value
The rope's flags.