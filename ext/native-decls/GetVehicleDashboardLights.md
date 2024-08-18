---
ns: CFX
apiset: client
game: gta5
---
## GET_VEHICLE_DASHBOARD_LIGHTS

```c
int GET_VEHICLE_DASHBOARD_LIGHTS();
```

Gets the state of the player vehicle's dashboard lights as a bit set
	indicator_left = 1
	indicator_right = 2
	handbrakeLight = 4
	engineLight = 8
	ABSLight = 16
	gasLight = 32
	oilLight = 64
	headlights = 128
	highBeam = 256
	batteryLight = 512