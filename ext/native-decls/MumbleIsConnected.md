---
ns: CFX
apiset: client
---
## MUMBLE_IS_CONNECTED

```c
BOOL MUMBLE_IS_CONNECTED();
```

## Return value
Returns `true` if the user is connected to the voice server, this will always return `false` if the user has voice chat disabled in their settings.
