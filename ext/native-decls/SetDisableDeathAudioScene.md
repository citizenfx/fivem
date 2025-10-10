---
ns: CFX
apiset: client
game: rdr3
---
## SET_DISABLE_DEATH_AUDIO_SCENE

```c
void SET_DISABLE_DEATH_AUDIO_SCENE(BOOL disable);
```

Disables the death audio scenes that normally mute all sounds except frontend when the player dies.
Specifically, it disables the audio scenes `0x8B8B8CB1 ` and `0x48A61B5F`.

## Parameters
* **disable**: Whether to disable the death audio scene.
