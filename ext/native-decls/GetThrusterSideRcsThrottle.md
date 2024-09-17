---
ns: CFX
apiset: server
game: gta5
---
## GET_THRUSTER_SIDE_RCS_THROTTLE

```c
float GET_THRUSTER_SIDE_RCS_THROTTLE(Vehicle jetpack);
```

## Parameters
* **jetpack**: The jetpack to check.

## Return value
Returns a value representing the side RCS (Reaction Control System) throttle of the jetpack. The values range from `0.0` (no throttle) to `1.0` (full throttle).