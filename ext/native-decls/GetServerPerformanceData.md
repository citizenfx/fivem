---
ns: CFX
apiset: server
---
## GET_SERVER_PERFORMANCE_DATA

```c
object GET_SERVER_PERFORMANCE_DATA();
```

Returns timing data for the server's main tick and resource execution. All durations are in milliseconds.

The data returned has the following layout:

```json
{
  "tickTime": 2.41,
  "tickTimeAverage": 2.73,
  "tickTimeMax": 8.91,
  "scriptTime": 1.52,
  "frameCount": 123456
}
```

`tickTime` is the duration of the last completed `svMain` tick. `tickTimeAverage` and `tickTimeMax` are calculated over up to 64 completed ticks. `scriptTime` is the duration of the last completed resource manager tick, including resource script execution. `frameCount` is the total number of observed `svMain` ticks since the server started.

Before the first server tick completes, all fields are zero.

## Return value
An object containing the latest server performance data.

## Examples

```lua
local performance = GetServerPerformanceData()

print(('Tick: %.2f ms (avg %.2f ms, max %.2f ms)'):format(
    performance.tickTime,
    performance.tickTimeAverage,
    performance.tickTimeMax
))
print(('Scripts: %.2f ms'):format(performance.scriptTime))
```

```js
const performance = GetServerPerformanceData();

console.log(`Tick: ${performance.tickTime.toFixed(2)} ms`);
console.log(`Scripts: ${performance.scriptTime.toFixed(2)} ms`);
```
