---
ns: CFX
apiset: client
game: gta5
aliases: ["GET_VEHICLE_DASHBOARD_WATER_TEMP"]
---
## GET_VEHICLE_DASHBOARD_CURRENT_GEAR

```c
float GET_VEHICLE_DASHBOARD_CURRENT_GEAR();
```

Retrieves the current gear displayed on the dashboard of the vehicle the player is in, returned as a float. This value represents the gear shown in the instrument cluster, such as "R" (0.0) or positive values (e.g., 1.0, 2.0, etc.) for drive gears.

## Return value
The current gear.
