---
ns: CFX
apiset: server
---
## SET_ENTITY_COORDS

```c
void SET_ENTITY_COORDS(Entity entity, float xPos, float yPos, float zPos, BOOL xAxis, BOOL yAxis, BOOL zAxis, BOOL clearArea);
```

p7 is always 1 in the scripts. Set to 1, an area around the destination coords for the moved entity is cleared from other entities.
Often ends with 1, 0, 0, 1); in the scripts. It works.
Axis - Invert Axis Flags

## Parameters
* **entity**: 
* **xPos**: 
* **yPos**: 
* **zPos**: 
* **xAxis**: 
* **yAxis**: 
* **zAxis**: 
* **clearArea**: 

