---
ns: CFX
apiset: client
---
## SET_DISCORD_RICH_PRESENCE_ASSET_TEXT

```c
void SET_DISCORD_RICH_PRESENCE_ASSET_TEXT(char* text);
```

This native sets the hover text of the image asset for the discord rich presence implementation.

## Parameters
* **text**: Text to be displayed when hovering over image asset. Note that you must also set a valid image asset using the SET_DISCORD_RICH_PRESENCE_ASSET native.