---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_DIRECTION

```c
void SET_LIGHT_DIRECTION(float xDir, float yDir, float zDir, float xTanDir, float yTanDir, float zTanDir);
```

Set the forward and tangent direction vectors for an existing light, allowing control over its orientation (useful for spotlights and directional lights).

## Parameters

* **xDir**, **yDir**, **zDir**: Components of the normalized forward (direction) vector
* **xTanDir**, **yTanDir**, **zTanDir**: Components of the normalized tangent vector (defines rotation around the forward axis)
