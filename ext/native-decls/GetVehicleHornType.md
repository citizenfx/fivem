---
ns: CFX
apiset: server
game: gta5
---
## GET_VEHICLE_HORN_TYPE

```c
Hash GET_VEHICLE_HORN_TYPE(Vehicle vehicle);
```

This is a getter for the client-side native [`START_VEHICLE_HORN`](https://docs.fivem.net/natives/?_0x9C8C6504B5B63D2C), which allows you to return the horn type of the vehicle.

**Note**: This native only gets the hash value set with `START_VEHICLE_HORN`. If a wrong hash is passed into `START_VEHICLE_HORN`, it will return this wrong hash.

```c
enum eHornTypes
{
    NORMAL = 1330140148,
    HELDDOWN = -2087385909,
    AGGRESSIVE = -92810745
}
```

## Parameters
* **vehicle**: The vehicle to check the horn type.

## Return value
Returns the vehicle horn type hash, or `0` if one is not set.