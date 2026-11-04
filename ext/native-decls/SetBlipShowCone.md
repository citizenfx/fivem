---
ns: CFX
apiset: client
game: gta5
---
## SET_BLIP_SHOW_CONE

```c
void SET_BLIP_SHOW_CONE(Blip blip, Bool toggle, Int hudColorIndex);
```

This function show or hide the cone of a blip, you can also set the cone color by using the last parameter "hudColorIndex"

## Parameters
* **blip**: The blip handle.
* **toggle**: "true" to show, "false" to hide.
* **hudColorIndex**: use the hud colors listed here : https://docs.fivem.net/docs/game-references/hud-colors/.
