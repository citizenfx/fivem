---
ns: CFX
apiset: client
game: gta5
---
## SET_WEATHER_INTERPOLATE

```c
void SET_WEATHER_INTERPOLATE(char* weatherTypePrev, char* weatherTypeNext, float interp);
```
## Examples

Valid weather types are:
* EXTRASUNNY
* CLEAR
* CLOUDS
* SMOG
* FOGGY
* OVERCAST
* RAIN
* THUNDER
* CLEARING
* NEUTRAL
* SNOW
* BLIZZARD
* SNOWLIGHT
* XMAS
* HALLOWEEN
* RAIN_HALLOWEEN (added in 3258)
* SNOW_HALLOWEEN (added in 3258)

## Parameters
* **weatherTypePrev**: any valid weather type
* **weatherTypeNext**: any valid weather type
* **interp**: float between 0.0-1.0