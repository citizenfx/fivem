---
ns: CFX
apiset: client
game: gta5
---
## SET_LIGHT_FALLOFF

```c
void SET_LIGHT_FALLOFF(int lightIndex, float falloff);
```

Adjust the falloff parameter for an existing light, affecting how light intensity decreases over distance.

## Parameters

* **lightIndex**: The index of the light created via `CreateLight`
* **falloff**: A floating‑point value determining the rate at which light intensity diminishes with distance (must be > 0; values ≤ 0 will be clamped internally)

## Return value
None.
