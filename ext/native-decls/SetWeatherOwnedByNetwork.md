---
ns: CFX
apiset: client
---
## SET_WEATHER_OWNED_BY_NETWORK

```c
void SET_WEATHER_OWNED_BY_NETWORK(BOOL network);
```

Sets whether or not the weather should be owned by the network subsystem.

To be able to use [_SET_WEATHER_TYPE_TRANSITION](#_0x578C752848ECFA0C), this has to be set to false.

## Parameters
* **network**: true to let the network control weather, false to not use network weather behavior.

