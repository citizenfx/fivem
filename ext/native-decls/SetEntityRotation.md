---
ns: CFX
apiset: server
---
## SET_ENTITY_ROTATION

```c
void SET_ENTITY_ROTATION(Entity entity, float pitch, float roll, float yaw, int rotationOrder, BOOL p5);
```

rotationOrder refers to the order yaw pitch roll is applied
value ranges from 0 to 5. What you use for rotationOrder when setting must be the same as rotationOrder when getting the rotation.
Unsure what value corresponds to what rotation order, more testing will be needed for that.
For the most part R* uses 1 or 2 as the order.
p5 is usually set as true

## Parameters
* **entity**: 
* **pitch**: 
* **roll**: 
* **yaw**: 
* **rotationOrder**: 
* **p5**: 

