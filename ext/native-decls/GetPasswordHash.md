---
ns: CFX
apiset: server
---
## GET_PASSWORD_HASH

```c
char* GET_PASSWORD_HASH(char* password);
```

**NOTE**: This cryptographic generation happens on the main thread, this should be used sparringly.


## Parameters
* **password**: The pasword to get the hash for

## Return value
Returns the password hashed with bcrypt
