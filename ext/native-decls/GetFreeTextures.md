---
ns: CFX
apiset: client
game: rdr3
---
## GET_FREE_TEXTURES

```c
int GET_FREE_TEXTURES();
```

Returns how many texture overrides can still be created in the global pool. Use this to check how many more textures can be added before the pool is exhausted.

## Returns
Free texture override count (`0`-`384`).
