---
ns: CFX
apiset: client
game: rdr3
---
## SET_ENTITY_LIGHT_COLOR

```c
int SET_ENTITY_LIGHT_COLOR(Entity entity, int red, int green, int blue);
```

Overrides the RGB color of the entity's model lights on this client, without changing other instances of the same model. Each channel must be between 0 and 255. Alpha, intensity and the engine's time-of-day behavior are preserved. This does not recolor emissive materials or particle effects.

Returns the number of model lights, or 0 for an invalid entity, an entity without model lights, or invalid color values.

Use `RESET_ENTITY_LIGHTS` to remove the overrides. Overrides are also removed when the entity is deleted, the client disconnects, or the resource that last set the entity's overrides stops. Changes are not network-replicated.
