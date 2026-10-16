---
ns: CFX
apiset: client
game: rdr3
---
## HAS_ENTITY_CUSTOM_LIGHTS

```c
bool HAS_ENTITY_CUSTOM_LIGHTS(Entity entity);
```

Returns true if the entity has a color or intensity override from `SET_ENTITY_LIGHT_COLOR` or `SET_ENTITY_LIGHT_INTENSITY` on this client. Returns false for an invalid entity or after `RESET_ENTITY_LIGHTS`.
