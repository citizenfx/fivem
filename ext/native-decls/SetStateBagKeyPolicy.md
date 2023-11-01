---
ns: CFX
apiset: server
---
## SET_STATE_BAG_KEY_POLICY

```c
void SET_STATE_BAG_KEY_POLICY(char* keyName, BOOL writePolicy);
```

Sets the write policy for a statebag key. 
(This affects all player and entity statebags.)

## Parameters
* **keyName**: The key name to set the write policy for.
* **writePolicy**: Whether to allow clients to write to this key.
