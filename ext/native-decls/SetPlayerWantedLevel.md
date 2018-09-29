---
ns: CFX
apiset: server
---
## SET_PLAYER_WANTED_LEVEL

```c
void SET_PLAYER_WANTED_LEVEL(Player player, int wantedLevel, BOOL disableNoMission);
```

Call SET_PLAYER_WANTED_LEVEL_NOW for immediate effect
wantedLevel is an integer value representing 0 to 5 stars even though the game supports the 6th wanted level but no police will appear since no definitions are present for it in the game files
disableNoMission-  Disables When Off Mission- appears to always be false

## Parameters
* **player**: 
* **wantedLevel**: 
* **disableNoMission**: 

