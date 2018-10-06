---
ns: CFX
apiset: client
---
## SET_DISCORD_RICH_PRESENCE_ASSET_SMALL_TEXT

```c
void SET_DISCORD_RICH_PRESENCE_ASSET_SMALL_TEXT(char* text);
```

This native sets the hover text of the small image asset for the discord rich presence implementation.

## Parameters
* **text**: Text to be displayed when hovering over small image asset. Note that you must also set a valid small image asset using the SET_DISCORD_RICH_PRESENCE_ASSET_SMALL native.