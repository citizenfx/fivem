---
ns: CFX
apiset: client
game: rdr3
---
## GET_ENTITY_LIGHT_COUNT

```c
int GET_ENTITY_LIGHT_COUNT(Entity entity);
```

Gets the number of lights defined in the entity's model light group. Returns 0 for an invalid entity or a model without lights. This does not count particle lights or lights drawn separately by scripts.
