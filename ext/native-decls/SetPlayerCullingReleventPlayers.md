---
ns: CFX
apiset: server
---
## SET_PLAYER_CULLING_RELEVANT_PLAYERS

```c
void SET_PLAYER_CULLING_RELEVANT_PLAYERS(char* playerSrc, int playerTarget);
```

Sets the culling relevent players list for the specified player.
Set to `0` to reset.

## Parameters
* **playerSrc**: The player to set the culling players for.
* **playerTarget**: The players you want cull. (You can pass many net ids)

## Examples

```lua
RegisterServerEvent("cullingPlayers")
AddEventHandler("cullingPlayers", function(list)
  local Source = source
  for i, v in ipairs(list) do
     list[i] = tonumber(v)
  end
  SetPlayerCullingRelevantPlayers(Source, table.unpack(list))
end)
```
