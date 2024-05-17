---
ns: CFX
apiset: client
game: gta5
---
## SET_FUEL_CONSUMPTION_RATE_MULTIPLIER

```c
void SET_FUEL_CONSUMPTION_RATE_MULTIPLIER(float multiplier);
```

Sets fuel consumption rate multiplier for all vehicles operated by a player. This is a way to slow down or speed up fuel consumption for all vehicles at a time. If 0 - it practically means that fuel will not be consumed. By default is set to 1.

When the multiplier is set to 1 a default 65 litre gas tank car with average fuel consumption can stay idle for ~16.67 hours or run with max RPM for ~2.5 hours.

To customize fuel consumption per vehicle / vehicle class use [`SET_HANDLING_FLOAT`](#_0x90DD01C)/[`SET_VEHICLE_HANDLING_FLOAT`](#_0x488C86D2) natives with `fieldName` equal to `fPetrolConsumptionRate`. By default it is set to 0.5 for all vehicles.

## Parameters
* **multiplier**: Global fuel consumption multiplier. If negative - 0 will be used instead.
