---
ns: CFX
apiset: client
game: gta5
---
## SET_PED_MODEL_HEALTH_CONFIG

```c
void SET_PED_MODEL_HEALTH_CONFIG(Hash modelHash, char* configName);
```

Sets a ped model's health config.
Takes effect only after setting player model with `SET_PLAYER_MODEL`.

## Examples
```lua
local pedModel = `mp_f_freemode_01`
SetPedModelHealthConfig(pedModel, "Strong")

SetPlayerModel(PlayerId(), pedModel)
SetPedDefaultComponentVariation(PlayerPedId())

```

## Parameters
* **modelHash**: Ped's model.
* **configName**: Name of health config.
