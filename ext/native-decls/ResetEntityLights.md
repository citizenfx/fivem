---
ns: CFX
apiset: client
game: rdr3
---
## RESET_ENTITY_LIGHTS

```c
void RESET_ENTITY_LIGHTS(Entity entity);
```

Removes the entity's color and intensity overrides on this client. Subsequent rendering uses the current shared model values, including any changes made through the original game natives. Has no effect for an invalid entity or an entity without overrides.
