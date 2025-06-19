---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_DIRECTION

```c
bool SET_LIGHT_DIRECTION(int lightIndex, float xDir, float yDir, float zDir, float xTanDir, float yTanDir, float zTanDir);
```

Set the forward and tangent direction vectors for an existing light, allowing control over its orientation (useful for spotlights and directional lights).

## Parameters

* **lightIndex**: The index of the created light
* **xDir**, **yDir**, **zDir**: Components of the normalized forward (direction) vector
* **xTanDir**, **yTanDir**, **zTanDir**: Components of the normalized tangent vector (defines rotation around the forward axis)

## Return value
A boolean indicating whether the operation succeeded.
