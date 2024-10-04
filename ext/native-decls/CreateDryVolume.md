---
ns: CFX
apiset: client
game: gta5
---
## CREATE_DRY_VOLUME

```c
int CREATE_DRY_VOLUME(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax);
```

Creates a volume where water effects do not apply. 
Useful for preventing water collisions from flooding areas underneath them.
This has no effect on waterquads, only water created from drawables and collisions.
Don't create volumes when your local ped is swimming (e.g. use IS_PED_SWIMMING in your scripts before you call this)

## Parameters
* **xMin**: The min X component of the AABB volume where water does not affect the player.
* **yMin**: The min Y component for the AABB volume.
* **zMin**: The min Z component for the AABB volume.
* **xMax**: The max X component for the AABB volume.
* **yMax**: The max Y component for the AABB volume.
* **zMax**: The max Z component for the AABB volume.

## Returns

The handle of the created volume.