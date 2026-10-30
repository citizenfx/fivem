---
ns: CFX
apiset: client
game: gta5
---
## SET_ENTITY_ROTATION_VELOCITY

```c
void SET_ENTITY_ROTATION_VELOCITY(Entity entity, float x, float y, float z);
```

Sets the angular (rotation) velocity of the specified entity.

Despite living alongside the vehicle natives, this one is not restricted to vehicles: the
entity is resolved from its handle and the call is dispatched through the entity's own
virtual method, so any entity handle is accepted.

Passing a handle that does not resolve to an entity does nothing and logs a message to the
console.

## Parameters
* **entity**: The entity to set the rotation velocity of.
* **x**: The angular velocity around the X axis.
* **y**: The angular velocity around the Y axis.
* **z**: The angular velocity around the Z axis.

## Examples
```lua
local vehicle = GetVehiclePedIsIn(PlayerPedId(), false)

-- spin the vehicle around its vertical axis
SetEntityRotationVelocity(vehicle, 0.0, 0.0, 2.0)
```

```js
const vehicle = GetVehiclePedIsIn(PlayerPedId(), false);

// spin the vehicle around its vertical axis
SetEntityRotationVelocity(vehicle, 0.0, 0.0, 2.0);
```
