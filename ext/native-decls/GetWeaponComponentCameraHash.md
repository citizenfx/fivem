---
ns: CFX
apiset: client
game: gta5
---
## GET_WEAPON_COMPONENT_CAMERA_HASH

```c
int GET_WEAPON_COMPONENT_CAMERA_HASH(Hash componentHash);
```

A getter for `CameraHash` in a weapon scope component.

```xml
<CameraHash>SNIPER_LOW_ZOOM_AIM_CAMERA</CameraHash>
```

## Parameters
* **componentHash**: Weapon component name hash.

## Return value
Returns the hash of the weapon scope.
