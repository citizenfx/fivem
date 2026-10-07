---
ns: CFX
apiset: server
---
## NETWORK_SET_FIRST_ENTITY_OWNER

```c
void NETWORK_SET_FIRST_ENTITY_OWNER(Entity entity, char* playerSrc);
```

**WARNING**: This native is not planned to be ported to Enhanced. You should make sure this native exists before calling it.

In Lua you can do this by doing `if NetworkSetFirstEntityOwner then` in JS you can do this by checking that `typeof globalThis.NetworkGetFirstEntityOwner === "function"`.

If you fail to do this when enhanced is released, it is likely that native will error and you will be expected to fix it.

**NOTE**: This only works for server created entities, and only work in the same tick that the server-created entity was made, trying to call this after an owner has been assigned will do nothing.

This native sets the first entity owner for server-created entitys in order to reduce the migration the server might do with the entity.

## Parameters
* **entity**: The entity to set the first owner for.
* **playerSrc**: The player to set as the owner

## Examples
```lua
-- for this example we just get the first player
local player = GetPlayers()[1]

-- create the helicopter
local heli = CreateVehicleServerSetter(`seasparrow`, 'heli', GetEntityCoords(GetPlayerPed(player)) + vector3(0, 0, 15), 0.0)

-- and set the first entity owner to the player
if NetworkSetFirstEntityOwner then
  NetworkSetFirstEntityOwner(heli, player)
end
```
