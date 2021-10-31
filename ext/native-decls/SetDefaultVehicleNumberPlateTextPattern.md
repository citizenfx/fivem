---
ns: CFX
apiset: client
game: gta5
---
## SET_DEFAULT_VEHICLE_NUMBER_PLATE_TEXT_PATTERN

```c
void SET_DEFAULT_VEHICLE_NUMBER_PLATE_TEXT_PATTERN(int plateIndex, char* pattern);
```

Sets the default number plate text pattern for vehicles seen on the local client with the specified plate index as their _default_ index (`plateProbabilities` from carvariations).

For consistency, this should be used with the same value on all clients, since vehicles _without_ custom text will use a seeded random number generator with this pattern to determine the default plate text.

The default value is `11AAA111`, and using this or a NULL string will revert to the default game RNG.

### Pattern string format

* `1` will lead to a random number from 0-9.
* `A` will lead to a random letter from A-Z.
* `.` will lead to a random letter _or_ number, with 50% probability of being either.
* `^1` will lead to a literal `1` being emitted.
* `^A` will lead to a literal `A` being emitted.
* Any other character will lead to said character being emitted.
* A string shorter than 8 characters will be padded on the right.

## Parameters
* **plateIndex**: A plate index, or `-1` to set a default for any indices that do not have a specific value.
* **pattern**: A number plate pattern string, or a null value to reset to default.

## Examples
```lua
SetDefaultVehicleNumberPlateTextPattern(-1, ' AAA111 ')
SetDefaultVehicleNumberPlateTextPattern(4 , ' AAAAAA ')

-- fixed characters: plate will be FAYUM69C for example
SetDefaultVehicleNumberPlateTextPattern(-1, 'F^AYUM11A')
```