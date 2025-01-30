---
ns: CFX
apiset: client
game: gta5
---
## DRAW_CORONA

```c
void DRAW_CORONA(float posX, float posY, float posZ, float size, int red, int green, int blue, int alpha, float intensity, float zBias, float dirX, float dirY, float dirZ, float viewThreshold, float innerAngle, float outerAngle, int flags);
```

Allows drawing advanced light effects, known as coronas, which support flares, volumetric lighting, and customizable glow properties.

## Parameters
* **posX**: The X position of the corona origin.
* **posY**: The Y position of the corona origin.
* **posZ**: The Z position of the corona origin.
* **size**: The size of the corona.
* **red**: The red component of the marker color, on a scale from 0-255.
* **green**: The green component of the marker color, on a scale from 0-255.
* **blue**: The blue component of the marker color, on a scale from 0-255.
* **alpha**: The alpha component of the marker color, on a scale from 0-255.
* **intensity**: The intensity of the corona.
* **zBias**: zBias slightly shifts the depth of surfaces to make sure they don’t overlap or cause visual glitches when they are very close together. The zBias value are usually in the range of 0.0 to 1.0.
* **dirX**: The X direction of the corona.
* **dirY**: The Y direction of the corona.
* **dirZ**: The Z direction of the corona.
* **viewThreshold**: The view threshold is to determine the fading of the corona based on the distance.
* **innerAngle**: The inner angle of the corona.
* **outerAngle**: The outer angle of the corona.
* **flags**: The corona flags.

```cpp
enum eCoronaFlags
{
    CF_HAS_DIRECTION = 2, // Only render the corona in the direction of dirX, dirY, dirZ
    CF_NO_RENDER_UNDERWATER = 4, // Disable rendering of the corona underwater
    CF_NO_RENDER_UNDERGROUND = 8, // This keep drawing the corona in underground zones (like tunnels)
    CF_NO_RENDER_FLARE = 16, // Disable rendering of the corona flare
}
```

## Examples

```lua
local pedCoords = GetEntityCoords(PlayerPedId())
Citizen.CreateThread(function()
    while true do
        DrawCorona(pedCoords.x, pedCoords.y, pedCoords.z, 5.0, 255, 255, 255, 255, 4.0, 0.2, pedCoords.x, pedCoords.y, pedCoords.z, 1.0, 0.0, 90.0, 2)
        Wait(0)
    end
end)
```
