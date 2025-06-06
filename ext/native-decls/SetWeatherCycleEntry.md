---
ns: CFX
apiset: client
---
## SET_WEATHER_CYCLE_ENTRY

```c
BOOL SET_WEATHER_CYCLE_ENTRY(int index, char* typeName, int timeMult);
```

## Examples

Sets the name and duration of a weather cycle. Changes are not applied until ['APPLY_WEATHER_CYCLES'](#_0x3422291C) is called.

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
* **index**: The index of the entry to set. Must be between 0 and 255
* **typeName**: The name of the weather type for this cycle
* **timeMult**: The relative duration of this cycle, which is multiplied by `msPerCycle` during ['APPLY_WEATHER_CYCLES'](#_0x3422291C). Must be between 1 and 255

## Return value
Returns true if all parameters were valid, otherwise false.