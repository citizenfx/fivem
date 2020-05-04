---
ns: CFX
apiset: server
---
## REQUEST_PLAYER_COMMERCE_SESSION

```c
void REQUEST_PLAYER_COMMERCE_SESSION(char* playerSrc, int skuId);
```

Requests the specified player to buy the passed SKU. This'll pop up a prompt on the client, which upon acceptance
will open the browser prompting further purchase details.

## Parameters
* **playerSrc**: The player handle
* **skuId**: The ID of the SKU.
