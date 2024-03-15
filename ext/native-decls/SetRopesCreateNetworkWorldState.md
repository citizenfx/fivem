---
ns: CFX
apiset: client
game: gta5
---
## SET_ROPES_CREATE_NETWORK_WORLD_STATE

```c
void SET_ROPES_CREATE_NETWORK_WORLD_STATE(BOOL shouldCreate);
```

Toggles whether the usage of [ADD_ROPE](#_0xE832D760399EB220) should create an underlying CNetworkRopeWorldStateData. By default this is set to false.

## Parameters
* **shouldCreate**: Whether to create an underlying network world state
