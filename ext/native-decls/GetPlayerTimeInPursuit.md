---
ns: CFX
apiset: server
---
## GET_PLAYER_TIME_IN_PURSUIT

```c
int GET_PLAYER_TIME_IN_PURSUIT(char* playerSrc, BOOL lastPursuit);
```

Gets the amount of time player has spent evading the cops.

The counter starts and increments only when cops are chasing the player.

If the player is evading (when the stars start to blink), the timer will pause.

## Parameters
* **playerSrc**: The target player
* **lastPursuit**: If set to true it will get the last persuit, false will get current

## Return value
Returns -1, if the player is not wanted or wasn't in pursuit before, depending on the `lastPursuit` parameter

Returns 0, if lastPursuit == False and the player has a wanted level, but the pursuit has not started yet

Otherwise, will return the milliseconds of the pursuit.
