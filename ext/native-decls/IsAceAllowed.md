---
ns: CFX
apiset: shared
---
## IS_ACE_ALLOWED

```c
BOOL IS_ACE_ALLOWED(char* object);
```

## Parameters
* **object**: The permission to check for, i.e. "command.ensure"

## Return value
For the server returns `true` if the specific script runtime has access to the specified ace permission
For the client this will always be `false` unless there is no ace for the specified object, since clients don't get ace permissions.
