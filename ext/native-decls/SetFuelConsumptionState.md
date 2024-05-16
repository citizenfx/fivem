---
ns: CFX
apiset: client
game: gta5
---
## SET_FUEL_CONSUMPTION_STATE

```c
void SET_FUEL_CONSUMPTION_STATE(BOOL state);
```

Turns on and off fuel consumption in all vehicles operated by a player. NPC operated vehicles will not consume fuel to avoid traffic disruptions.

The default Gta5 behaviour is fuel consumption turned off.

## Parameters
* **state**: True to turn on. False to turn off.
