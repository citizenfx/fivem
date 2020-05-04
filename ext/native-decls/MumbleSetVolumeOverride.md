---
ns: CFX
apiset: client
---
## MUMBLE_SET_VOLUME_OVERRIDE

```c
void MUMBLE_SET_VOLUME_OVERRIDE(Player player, float volume);
```

Overrides the output volume for a particular player on Mumble. This will also bypass 3D audio and distance calculations. -1.0 to reset the override.

Set to -1.0 to reset the Volume override.

## Parameters
* **player**: A game player index.
* **volume**: The volume, ranging from 0.0 to 1.0 (or above).

