---
ns: CFX
apiset: client
game: gta5
---
## GET_WEAPON_COMPONENT_CLIP_SIZE

```c
int GET_WEAPON_COMPONENT_CLIP_SIZE(Hash componentHash);
```

A getter for `ClipSize` in a weapon component.

```xml
<ClipSize value="30" />
```

## Parameters
* **componentHash**: Weapon component name hash.

## Return value
Returns the weapon components clip size.
