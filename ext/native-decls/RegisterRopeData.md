---
ns: CFX
apiset: client
game: gta5
---
## REGISTER_ROPE_DATA

```c
int REGISTER_ROPE_DATA(int numSections, float radius, char* diffuseTextureName, char* normalMapName, float distanceMappingScale, float uvScaleX, float uvScaleY, float specularFresnel, float specularFalloff, float specularIntensity, float bumpiness, int color);
```

Registers a custom rope data with the game. For guidance on what these values should be use common:/data/ropedata.xml as a reference.
Returns a rope type which can be passed into [ADD_ROPE](#_0xE832D760399EB220) to use a custom rope design.
Once a rope data is registered it can be used indefinitely and you should take caution not too register too many as to exceed the games limit.

## Examples

```lua
-- Create a thick steel cable rope above the players head
local ropeType = RegisterRopeData(6, 0.15, "steel_cable", "steel_cable_n", 1.0, 1.0, 8.775, 0.97, 30.0, 0.25, 1.775, 0x00FFFF00)
if ropeType ~= -1 then
    local coords = GetEntityCoords(PlayerPedId()) + vector3(0.0, 0.0, 5.0)
	AddRope(coords.x, coords.y, coords.z, 0.0, 0.0, 0.0, 25.0, ropeType, 10.0, 0.0, 1.0, false, false, false, 1.0, false, 0)
    RopeLoadTextures()
end
```

## Parameters
* **numSections**:
* **radius**:
* **diffuseTextureName**:
* **normalMapName**:
* **uvScaleX**:
* **uvScaleY**:
* **specularFresnel**:
* **specularFalloff**:
* **specularIntensity**:
* **bumpiness**:
* **color**:

## Return value
Returns a non-negative value on success, or -1 if the rope data could not be registered or an invalid argument is passed.