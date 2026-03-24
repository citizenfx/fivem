---
ns: CFX
apiset: client
game: rdr3
---
## GET_CLIENT_CONFIG_BOOL

```c
BOOL GET_CLIENT_CONFIG_BOOL(int flagIndex);
```

Returns whether a specific client configuration flag is currently enabled.
You can find a list of configuration flags in [`SET_CLIENT_CONFIG_BOOL`](#_0xD174EF7E).


## Parameters
* **flagIndex**: The index of the configuration flag to get.