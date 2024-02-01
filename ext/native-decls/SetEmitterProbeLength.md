---
ns: CFX
apiset: client
game: gta5
---
## SET_EMITTER_PROBE_LENGTH

```c
void SET_EMITTER_PROBE_LENGTH(float probeLength);
```

Allows StaticEmitter's without a linked entity to make use of environment features like occlusion and reverb even if they are located higher than 20.0 units above any static collision inside interiors.

This native allows you to extend the probe range up to 150.0 units.

## Examples

```lua
RegisterCommand("setEmitterProbeLength", function(src, args, raw)
    local probeLength = (tonumber(args[1]) + 0.0)

    print("Extending emitter probes to: ", probeLength)
    SetEmitterProbeLength(probeLength)
end)

RegisterCommand("resetEmitterProbeLength", function()
    print("Resetting emitter probes to default settings")
    SetEmitterProbeLength(20.0)
end)
```


## Parameters
* **probeLength**: The desired probe length (20.0 - 150.0)