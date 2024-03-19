---
ns: CFX
apiset: client
---
## GET_WORLD_COORD_FROM_SCREEN_COORD

```c
void GET_WORLD_COORD_FROM_SCREEN_COORD(float screenX, float screenY, Vector3* worldVector, Vector3* normalVector);
```

Converts a screen coordinate into its relative world coordinate.

## Examples

```lua
CreateThread(function()
  while true do
    local screenX = GetDisabledControlNormal(0, 239)
    local screenY = GetDisabledControlNormal(0, 240)

    local world, normal = GetWorldCoordFromScreenCoord(screenX, screenY)

    local depth = 10

    local target = world + normal * depth

    DrawSphere(target.x, target.y, target.z, 0.5, 255, 0, 0, 0.5)

    Wait(0)
  end
end)
```

## Parameters

- **screenX**: A screen horizontal axis coordinate (0.0 - 1.0).
- **screenY**: A screen vertical axis coordinate (0.0 - 1.0).
- **worldVector**: The world coord vector pointer.
- **normalVector**: The screen normal vector pointer.

## Return value

A Vector3 representing the world coordinates relative to the specified screen coordinates and a screen plane normal Vector3 (normalised).
