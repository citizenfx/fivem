---
ns: CFX
apiset: client
---
## APPLY_WEATHER_CYCLES

```c
BOOL APPLY_WEATHER_CYCLES(int numEntries, int msPerCycle);
```

## Examples

Overrides the weather cycles, using the values set through previous calls to ['SET_WEATHER_CYCLE_ENTRY'](#_0xD264D4E1).
Once applied, the list of cycles will run sequentially, and repeat after reaching the end.

```lua
-- Cycle between XMAS weather for 30 seconds (3 * 10000 milliseconds), and SMOG weather for 20 seconds (2 * 10000 milliseconds)
local success = SetWeatherCycleEntry(0, "XMAS", 3) and
                SetWeatherCycleEntry(1, "SMOG", 2) and
                ApplyWeatherCycles(2, 10000)
```

## Parameters
* **numEntries**: The number of cycle entries. Must be between 1 and 256
* **msPerCycle**: The duration in milliseconds of each cycle. Must be between 1000 and 86400000 (24 hours)

## Return value
Returns true if all parameters were valid, otherwise false.