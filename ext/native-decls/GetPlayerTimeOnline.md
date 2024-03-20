---
ns: CFX
apiset: server
---
## GET_PLAYER_TIME_ONLINE

```c
int GET_PLAYER_TIME_ONLINE(char* playerSrc);
```

Gets the current time online for a specified player.

## Parameters
* **playerSrc**: A player.

## Return value

The current time online in seconds.

## Examples

```lua
local function ShowTimeOnline()
    local player = source
    local secondsTotalOnline = GetPlayerTimeOnline(player)

    print(("Time online : %f H %f min %f"):format(
        (secondsTotalOnline / 3600),
        ((secondsTotalOnline / 60) % 60),
        (secondsTotalOnline % 60)
    ))
end

RegisterNetEvent("myTimeOnline", ShowTimeOnline)
```
