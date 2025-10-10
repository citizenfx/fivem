---
ns: CFX
apiset: client
game: rdr3
---
## GET_BLIP_ROTATION

```c
float GET_BLIP_ROTATION(int blipHandle);
```

Gets the rotation of a blip from its handle.

## Parameters
* **blipHandle**: The blip handle obtained from GET_GAME_POOL("CBlip")

## Return value
The blip rotation in degrees as a float value (0.0 to 360.0). Returns 0.0 if the blip doesn't exist.

## Examples
```lua
local blips = GetGamePool("CBlip")
for _, blipHandle in ipairs(blips) do
    local rotation = GetBlipRotation(blipHandle)
    print(string.format("Blip %d rotation: %.2f degrees", blipHandle, rotation))
end
```

```js
const blips = GetGamePool("CBlip");
blips.forEach(blipHandle => {
    const rotation = GetBlipRotation(blipHandle);
    console.log(`Blip ${blipHandle} rotation: ${rotation.toFixed(2)} degrees`);
});
```
