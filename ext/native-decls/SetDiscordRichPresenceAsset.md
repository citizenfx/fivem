---
ns: CFX
apiset: client
---
## SET_DISCORD_RICH_PRESENCE_ASSET

```c
void SET_DISCORD_RICH_PRESENCE_ASSET(char* assetName);
```

This native sets the image asset for the discord rich presence implementation.

## Parameters
* **assetName**: The name of a valid asset registered on Discordapp's developer dashboard. note that the asset has to be registered under the same discord API application set using the SET_DISCORD_APP_ID native.

