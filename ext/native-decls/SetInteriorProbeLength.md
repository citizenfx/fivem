---
ns: CFX
apiset: client
game: gta5
---
## SET_INTERIOR_PROBE_LENGTH

```c
void SET_INTERIOR_PROBE_LENGTH(float probeLength);
```

Overwrite the games default CPortalTracker interior detection range.
This fixes potentially unwanted behaviour in the base game and allows you to build custom interiors with larger ceiling heights without running into graphical glitches.

By default CPortalTracker will probe 4 units downward trying to reach collisions that are part of the interior the entity is in.
If no collision can be found 16 units are used in some circumstances.

There are 30+ hard coded special cases, only some of them exposed via script (for example `ENABLE_STADIUM_PROBES_THIS_FRAME`).

This native allows you to extend the probe range up to 150 units which is the same value the game uses for the `xs_arena_interior`

## Examples

```lua
RegisterCommand("setInteriorProbeLength", function(src, args, raw)
    local probeLength = (tonumber(args[1]) + 0.0)

    print("Extending interior detection probes to: ", probeLength)
    SetInteriorProbeLength(probeLength)
end)

RegisterCommand("resetInteriorProbeLength", function()
    print("Resetting interior detection probes to default settings")
    SetInteriorProbeLength(0.0)
end)
```


## Parameters
* **probeLength**: The desired probe length (0.0 - 150.0)