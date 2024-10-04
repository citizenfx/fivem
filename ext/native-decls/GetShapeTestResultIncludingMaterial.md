---
ns: CFX
apiset: client
game: rdr3
---
## GET_SHAPE_TEST_RESULT_INCLUDING_MATERIAL

```c
int GET_SHAPE_TEST_RESULT_INCLUDING_MATERIAL(int shapeTestHandle, BOOL* hit, Vector3* endCoords, Vector3* surfaceNormal, Hash* materialHash, Entity* entityHit);
```

Returns the result of a shape test, also returning the material of any touched surface.

When used with an asynchronous shape test, this native should be looped until returning 0 or 2, after which the handle is invalidated.

Unless the return value is 2, the other return values are undefined.

## Parameters
* **shapeTestHandle**: A shape test handle.
* **hit**: Whether or not the shape test hit any collisions.
* **endCoords**: The resulting coordinates where the shape test hit a collision.
* **surfaceNormal**: The surface normal of the hit position.
* **materialHash**: hash of the hit material or surface type, see materialFX.dat
* **entityHit**: Any dynamic entity hit by the shape test.

## Return value
`0` if the handle is invalid, `1` if the shape test is still pending, or `2` if the shape test has completed, and the handle should be invalidated.
