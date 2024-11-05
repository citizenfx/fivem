---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_DASHBOARD_LIGHTS

```c
int GET_VEHICLE_DASHBOARD_LIGHTS();
```

```c
enum eDashboardLights {
	IndicatorLeft = 1,
	IndicatorRight = 2,
	HandbrakeLight = 4,
	EngineLight = 8,
	ABSLight = 16,
	GasLight = 32,
	OilLight = 64,
	Headlights = 128,
	HighBeam = 256,
	BatteryLight = 512
}
```

## Return value
Returns the state of the vehicle dashboard lights

## Examples
```lua
local eDashboardLights = {
    IndicatorLeft = 1,
    IndicatorRight = 2,
    HandbrakeLight = 4,
    EngineLight = 8,
    ABSLight = 16,
    GasLight = 32,
    OilLight = 64,
    Headlights = 128,
    HighBeam = 256,
    BatteryLight = 512
}

local dashboardLights = GetVehicleDashboardLights()

if (dashboardLights & eDashboardLights.HighBeam) ~= 0 then
    print("The high beams are on!")
end

if (dashboardLights & eDashboardLights.GasLight) ~= 0 then
    print("You might need to get some fuel!")
end
```
