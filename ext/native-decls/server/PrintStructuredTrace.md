---
ns: CFX
apiset: server
---
## PRINT_STRUCTURED_TRACE

```c
void PRINT_STRUCTURED_TRACE(char* jsonString);
```

Prints 'structured trace' data to the server `file descriptor 3` channel. This is not generally useful outside of
server monitoring utilities.

## Parameters
* **jsonString**: JSON data to submit as `payload` in the `script_structured_trace` event.
