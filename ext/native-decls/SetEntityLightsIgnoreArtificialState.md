---
ns: CFX
apiset: client
game: gta5
---
## SET_ENTITY_LIGHTS_IGNORE_ARTIFICIAL_STATE

```c
void SET_ENTITY_LIGHTS_IGNORE_ARTIFICIAL_STATE(Entity entity, BOOL toggle);
```

Sets whether an entity's lights should ignore the artificial lights blackout state.
When `SET_ARTIFICIAL_LIGHTS_STATE(true)` is active (blackout mode), entities marked with this native will keep their lights on.

## Parameters
* **entity**: The entity handle.
* **toggle**: `true` to make the entity's lights ignore blackout, `false` to restore default behavior.

## Examples
```lua
-- Spawn a vehicle
local vehicle = CreateVehicle(modelHash, x, y, z, heading, true, false)

-- Enable blackout
SetArtificialLightsState(true)

-- This specific vehicle keeps its lights on during blackout
SetEntityLightsIgnoreArtificialState(vehicle, true)
```
