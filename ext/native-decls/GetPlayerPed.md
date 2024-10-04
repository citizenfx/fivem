---
ns: CFX
apiset: server
---
## GET_PLAYER_PED

```c
Entity GET_PLAYER_PED(char* playerSrc);
```

Used to get the player's Ped Entity ID when a valid `playerSrc` is passed.


## Parameters
* **playerSrc**: The player source, passed as a string.

## Return value
Returns a valid Ped Entity ID if the passed `playerSrc` is valid, `0` if not.

## Examples

```lua
-- Let's assume source is a valid ID
local pedId = GetPlayerPed(source);

-- If pedId is valid (not equal to 0)
if pedId ~= 0 then
    -- Do something with this ped!
end
```