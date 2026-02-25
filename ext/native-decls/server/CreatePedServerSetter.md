---
ns: CFX
apiset: server
---
## CREATE_PED_SERVER_SETTER

```c
Ped CREATE_PED_SERVER_SETTER(int pedType, Hash modelHash, float x, float y, float z, float heading, BOOL isNetwork, BOOL bScriptHostPed);
```

Equivalent to CREATE_PED, but it uses 'server setter' logic as a workaround for
reliability concerns regarding entity creation RPC.

Creates a ped (biped character, pedestrian, actor) with the specified model at the specified position and heading.

## Parameters
* **pedType**: Unused. Peds get set to CIVMALE/CIVFEMALE/etc. no matter the value specified.
* **modelHash**: The model of ped to spawn.
* **x**: Spawn coordinate X component.
* **y**: Spawn coordinate Y component.
* **z**: Spawn coordinate Z component.
* **heading**: Heading to face towards, in degrees.
* **isNetwork**: Whether to create a network object for the ped. If false, the ped exists only locally.
* **bScriptHostPed**: Whether to register the ped as pinned to the script host in the R* network model.

## Return value
A script handle (fwScriptGuid index) for the ped, or `0` if the ped failed to be created.