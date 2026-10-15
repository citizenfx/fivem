---
ns: CFX
apiset: shared
---
## CREATE_COLSHAPE_POLYGON

```c
int CREATE_COLSHAPE_POLYGON(float minZ, float maxZ, object points);
```

Creates a polygon collision shape defined by a set of 2D points extruded between `minZ` and `maxZ`. Requires at least 3 points.

When an entity enters or leaves a collision shape, the `onColshapeEnter` and `onColshapeExit` events are triggered. Both events are called with the entity handle that triggered them and the collision shape ID.

## Parameters
* **minZ**: Minimum Z height of the polygon.
* **maxZ**: Maximum Z height of the polygon.
* **points**: An array of interleaved x, y point pairs (e.g., `{x1, y1, x2, y2, x3, y3, ...}`). Must contain at least 3 pairs.

## Return value
The collision shape ID, or -1 on failure.

## Examples
```lua
local colShape = CreateColshapePolygon(28.0, 32.0, { 293.089, 180.466, 303.089, 180.466, 298.089, 190.466 })
print('created colshape with id ' .. tostring(colShape))

AddEventHandler('onColshapeEnter', function(entity, shape)
    print('entity ' .. tostring(entity) .. ' entered colshape ' .. tostring(shape))
end)

AddEventHandler('onColshapeExit', function(entity, shape)
    print('entity ' .. tostring(entity) .. ' left colshape ' .. tostring(shape))
end)
```
