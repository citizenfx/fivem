---
ns: CFX
apiset: server
---
## GET_ENTITY_HEALTH

```c
int GET_ENTITY_HEALTH(Entity entity);
```

## Parameters
* **entity**: The entity to check the health of

## Supported Entity Types

| Entity Type | FiveM | RedM  | Returns (FiveM) | Returns (RedM) |
|-------------|-------|-------|-----------------|----------------|
| Vehicle     | ✅    | ❌   | 0 - 1000        | 0             |
| Ped         | ✅    | ✅   | 0 - 200         | 0 - 150       |
| Object      | ✅    | ❌   | 0               | 0             |
| (Others)    | ❌    | ❌   | 0               | 0             |