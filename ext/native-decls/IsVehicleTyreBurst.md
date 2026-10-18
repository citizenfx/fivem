---
ns: CFX
apiset: server
---
## IS_VEHICLE_TYRE_BURST

```c
BOOL IS_VEHICLE_TYRE_BURST(Vehicle vehicle, int wheelID, BOOL completely);
```

```
The server side server-side RPC native equivalent has the wheelIDs of 2 and 4 as well as 3 and 5 flipped.

Since there is not a serverside of SetVehicleTyreBurst the ids just need to be flipped so they can be ID correctly on the client:

lua Example:
damages.tyr[2], damages.tyr[4] = damages.tyr[4], damages.tyr[2] -- Tire 4 from client is tire 2 on server
damages.tyr[3], damages.tyr[5] = damages.tyr[5], damages.tyr[3] -- Tire 5 from client is tire 3 on server
```

## Parameters
* **vehicle**: 
* **wheelID**: 
* **completely**: 

## Return value
