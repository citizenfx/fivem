---
ns: CFX
apiset: client
game: rdr3
---
## GET_ENTITY_LIGHT_COLOR

```c
void GET_ENTITY_LIGHT_COLOR(Entity entity, int lightIndex, int* red, int* green, int* blue);
```

Writes the RGB color of a model light to the output parameters. Indices start at 0; see `GET_ENTITY_LIGHT_COUNT`. Reads the entity's color override if set, otherwise the current shared model color, before timecycle modulation. Writes 0, 0, 0 for an invalid entity or light index.
