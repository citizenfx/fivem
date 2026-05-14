---
ns: CFX
apiset: server
---
## REQUEST_PLAYER_COMMERCE_SESSION

```c
void REQUEST_PLAYER_COMMERCE_SESSION(char* playerSrc, int skuId);
```

> This native is deprecated and may be removed in a future version. Use the [Tebex API](https://docs.tebex.io/) instead.

Requests the specified player to buy the passed SKU. This'll pop up a prompt on the client, which upon acceptance
will open the browser prompting further purchase details.

## Parameters
* **playerSrc**: The player handle
* **skuId**: The ID of the SKU.
