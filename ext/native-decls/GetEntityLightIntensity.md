---
ns: CFX
apiset: client
game: rdr3
---
## GET_ENTITY_LIGHT_INTENSITY

```c
float GET_ENTITY_LIGHT_INTENSITY(Entity entity, int lightIndex);
```

Gets the base intensity of a model light. Indices start at 0; see `GET_ENTITY_LIGHT_COUNT`. Reads the entity's intensity override if set, otherwise the current shared model intensity, before timecycle modulation. Returns 0.0 for an invalid entity or light index.
