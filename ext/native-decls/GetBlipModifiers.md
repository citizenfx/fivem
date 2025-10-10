---
ns: CFX
apiset: client
game: rdr3
---
## GET_BLIP_MODIFIERS

```c
object GET_BLIP_MODIFIERS(int blipHandle);
```

Gets the modifiers array of a blip from its handle.

## Parameters
* **blipHandle**: The blip handle obtained from GET_GAME_POOL("CBlip")

## Return value
An array of uint32_t modifiers. Returns an empty array if the blip doesn't exist.

## Examples
```lua
local blips = GetGamePool("CBlip")
for _, blipHandle in ipairs(blips) do
    local modifiers = GetBlipModifiers(blipHandle)
    if #modifiers > 0 and modifiers[1] == 0x9D6AB6CB then -- BLIP_STYLE_POI
        print("Found POI style blip with handle:", blipHandle)
    end
end
```

```js
const blips = GetGamePool("CBlip");
blips.forEach(blipHandle => {
    const modifiers = GetBlipModifiers(blipHandle);
    if (modifiers.length > 0 && modifiers[0] === 0x9D6AB6CB) { // BLIP_STYLE_POI
        console.log(`Found POI style blip with handle: ${blipHandle}`);
    }
});
```
