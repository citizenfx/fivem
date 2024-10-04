---
ns: CFX
apiset: client
game: gta5
---
## GET_RUNTIME_TEXTURE_PITCH

```c
int GET_RUNTIME_TEXTURE_PITCH(long tex);
```

Gets the row pitch of the specified runtime texture, for use when creating data for `SET_RUNTIME_TEXTURE_ARGB_DATA`.

## Parameters
* **tex**: A handle to the runtime texture.

## Return value
The row pitch in bytes.
