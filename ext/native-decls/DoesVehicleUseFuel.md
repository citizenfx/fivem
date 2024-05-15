---
ns: CFX
apiset: client
game: gta5
---
## DOES_VEHICLE_USE_FUEL

```c
BOOL DOES_VEHICLE_USE_FUEL(Vehicle vehicle);
```

Checks whether the vehicle consumes fuel. The check is done based on petrol tank volume and vehicle type. Bicycles and vehicles with petrol tank volume equal to zero (only bicycles by default) do not use fuel. All other vehicles do.

You can customize petrol tank volume using [`SET_HANDLING_FLOAT`](#_0x90DD01C)/[`SET_VEHICLE_HANDLING_FLOAT`](#_0x488C86D2) natives with `fieldName` equal to `fPetrolTankVolume`.

## Parameters
* **vehicle**: The vehicle handle.

## Return value
True if the vehicle uses fuel to move. False otherwise.
