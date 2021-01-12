---
ns: CFX
apiset: server
---
## SET_ROUTING_BUCKET_ENTITY_LOCKDOWN_MODE

```c
void SET_ROUTING_BUCKET_ENTITY_LOCKDOWN_MODE(int bucketId, char* mode);
```

Sets the entity lockdown mode for a specific routing bucket.

Lockdown modes are:

| Mode       | Meaning                                                    |
| ---------- | ---------------------------------------------------------- |
| `strict`   | No entities can be created by clients at all.              |
| `relaxed`  | Only script-owned entities created by clients are blocked. |
| `inactive` | Clients can create any entity they want.                   |

## Parameters
* **bucketId**: The routing bucket ID to adjust.
* **mode**: One of aforementioned modes.

