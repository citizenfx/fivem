---
ns: CFX
apiset: shared
---
## GET_CONSOLE_BUFFER

```c
char* GET_CONSOLE_BUFFER(int maxLines);
```

Returns the current console output buffer.

On the **server** this returns the recent stdout/stderr text. On the **client**
this returns the contents of the in-game F8 console (lines joined by newlines,
each line prefixed with `[channel] ` when the channel is set — typically
`[script:resource_name]` for resource prints).

## Parameters
- **maxLines**: Maximum number of trailing lines to return. Pass `0` (or any non-positive value) to retrieve the entire buffer.

## Return value
The most recent game console output, as a string.