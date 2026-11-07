---
ns: CFX
apiset: client
game: gta5
---
## GET_ALL_ENTITIES_IGNORING_ARTIFICIAL_LIGHTS_STATE

```c
object GET_ALL_ENTITIES_IGNORING_ARTIFICIAL_LIGHTS_STATE();
```

Returns all entity handles that are currently set to ignore the artificial lights blackout state.

## Return value
An array of entity handles.

## Examples
```lua
local entities = GetAllEntitiesIgnoringArtificialLightsState()
for _, entity in ipairs(entities) do
    print("Entity ignoring blackout: " .. entity)
end
```
