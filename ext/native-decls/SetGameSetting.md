---
ns: CFX
apiset: client
game: gta5
---
## SET_GAME_SETTING

```c
void SET_GAME_SETTING(char* name, float value);
```

Stages a change to one of the game's graphics or video settings. Staged changes take effect once
[APPLY_GAME_SETTINGS](#_0xB0FE5426) is called, so several settings can be changed at once.

Integer settings are truncated, and boolean settings are `true` for any non-zero value.

## Parameters
* **name**: The name of the setting, as returned by [GET_GAME_SETTINGS](#_0x3AB4EDCA), such as `TextureQuality`.
* **value**: The new value.
