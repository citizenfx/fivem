---
ns: CFX
apiset: server
---
## GET_VEHICLE_NUMBER_PLATE_TEXT

```c
char* GET_VEHICLE_NUMBER_PLATE_TEXT(Vehicle vehicle);
```

## Parameters
* **vehicle**: 

## Note
If the number plate is less than 8 character return value will have a space at the end to fill the remaining characters

## Return value
