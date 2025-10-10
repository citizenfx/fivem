---
ns: CFX
apiset: client
game: rdr3
---
## GET_BLIP_SPRITE

```c
Hash GET_BLIP_SPRITE(int blipHandle);
```

Gets the sprite/hash of a blip from its handle.

## Parameters
* **blipHandle**: The blip handle obtained from GET_GAME_POOL("CBlip")

## Return value
The blip sprite hash. Returns 0 if the blip doesn't exist.

## Examples
```lua
local blips = GetGamePool("CBlip")
for _, blipHandle in ipairs(blips) do
    local sprite = GetBlipSprite(blipHandle)
    if sprite == 0x866B73BE then -- BLIP_POI
        print("Found POI blip with handle:", blipHandle)
    end
end
```

```js
const blips = GetGamePool("CBlip");
blips.forEach(blipHandle => {
    const sprite = GetBlipSprite(blipHandle);
    if (sprite === 0x866B73BE) { // BLIP_POI
        console.log(`Found POI blip with handle: ${blipHandle}`);
    }
});
```
