---
ns: CFX
apiset: client
game: gta5
---
## APPLY_GAME_SETTINGS

```c
int APPLY_GAME_SETTINGS(BOOL save);
```

Applies changes staged through [SET_GAME_SETTING](#_0xE19400D5), the same way the pause menu applies them. Changes
take effect on the next frame; resolution, window mode and similar changes reset the graphics device.

Like the pause menu, changes the game only picks up on restart (such as `TextureQuality` or `ShaderQuality`, see
[GET_GAME_SETTING_REQUIRES_RESTART](#_0x0329588A)) are not applied live. If any staged change requires a restart, none
of the staged changes are applied live, and the result is `2`.

Restart-only changes therefore need `save` to have any effect: they're picked up from `settings.xml` when the game
starts. Without `save`, they're discarded.

The staged changes are discarded, and nothing is applied or saved, if the resulting resolution is not one of
[GET_AVAILABLE_SCREEN_RESOLUTIONS](#_0x095DBFD7) for the target monitor.

Changes are only kept for the current server session: when the player leaves the server, settings changed through
this native are restored to their previous values (unless the player changed them again in the meantime, or they
were saved).

Saving is subject to the player's consent: the first time a server asks to save, the player is prompted to allow or
deny it, and that choice is remembered for this server until the game is restarted. While the prompt is open, or if
the player denies it, the settings are not saved (but are still applied live where possible).

## Parameters
* **save**: Whether to also write the new settings to `settings.xml`, so they persist across server sessions and game
  restarts. Requires the player's consent.

## Return value
* `0`: The resolution is not available, nothing was applied.
* `1`: The settings were applied.
* `2`: The settings require a game restart, and were not applied.

## Examples
```lua
SetGameSetting('ScreenWidth', 1920)
SetGameSetting('ScreenHeight', 1080)
SetGameSetting('MSAA', 4)

local result = ApplyGameSettings(true)

if result == 0 then
    print('That resolution is not supported on this monitor.')
elseif result == 2 then
    print('These settings will take effect after restarting the game.')
end
```
