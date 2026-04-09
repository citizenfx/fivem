---
ns: CFX
apiset: client
game: rdr3
---
## SET_CLIENT_CONFIG_BOOL

```c
void SET_CLIENT_CONFIG_BOOL(int flagIndex, BOOL enabled);
```

```c
enum ClientConfigFlag
{
    WeaponsNoAutoReload = 0,
	UIVisibleWhenDead = 1,
	DisableDeathAudioScene = 2,
	DisableRemoteAttachments = 3
}
```

Sets the value of a client configuration flag.
This native allows enabling or disabling specific one-time client-side features.

## Parameters
* **flagIndex**: The index of the configuration flag to set.
* **enabled**: Whether to enable or disable the flag.