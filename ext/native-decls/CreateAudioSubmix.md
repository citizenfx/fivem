---
ns: CFX
apiset: client
---
## CREATE_AUDIO_SUBMIX

```c
int CREATE_AUDIO_SUBMIX(char* name);
```

Creates an audio submix with the specified name, or gets the existing audio submix by that name.

## Parameters
* **name**: The audio submix name.

## Return value
A submix ID, or -1 if the submix could not be created.
