---
ns: CFX
apiset: client
game: gta5
---
## GET_GAME_SETTING_REQUIRES_RESTART

```c
BOOL GET_GAME_SETTING_REQUIRES_RESTART(char* name);
```

Returns whether changing a setting requires a game restart, using the same check as the pause menu. Changes to these
settings are not applied live by [APPLY_GAME_SETTINGS](#_0xB0FE5426).

Some settings depend on the current state; for example, changing `AdapterIndex` only requires a restart in fullscreen
mode.

## Parameters
* **name**: The name of the setting, as returned by [GET_GAME_SETTINGS](#_0x3AB4EDCA), such as `TextureQuality`.

## Return value
Whether changing the setting requires a game restart.
