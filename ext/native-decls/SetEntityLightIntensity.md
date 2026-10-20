---
ns: CFX
apiset: client
game: rdr3
---
## SET_ENTITY_LIGHT_INTENSITY

```c
int SET_ENTITY_LIGHT_INTENSITY(Entity entity, float intensity);
```

Overrides the base intensity of the entity's model lights on this client, without changing other instances of the same model. Intensity must be finite and non-negative; 0 disables the light contribution. Color and the engine's time-of-day behavior are preserved. This does not change emissive materials or particle effects.

Returns the number of model lights, or 0 for an invalid entity, an entity without model lights, or an invalid intensity.

Use `RESET_ENTITY_LIGHTS` to remove the overrides. Overrides are also removed when the entity is deleted, the client disconnects, or the resource that last set the entity's overrides stops. Changes are not network-replicated.
