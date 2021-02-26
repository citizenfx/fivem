---
ns: CFX
apiset: client
---
## SET_DISCORD_RICH_PRESENCE_ACTION

```c
void SET_DISCORD_RICH_PRESENCE_ACTION(int index, char* label, char* url);
```

Sets a clickable button to be displayed in a player's Discord rich presence.

## Parameters
* **index**: The button index, either 0 or 1.
* **label**: The text to display on the button.
* **url**: The URL to open when clicking the button. This has to start with `fivem://connect/` or `https://`.

