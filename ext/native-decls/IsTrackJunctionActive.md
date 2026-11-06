---
ns: CFX
apiset: client
game: gta5
---
## IS_TRACK_JUNCTION_ACTIVE

```c
bool IS_TRACK_JUNCTION_ACTIVE(int junctionId, bool* isActive);
```

## Examples

```lua
local junctionId = RegisterTrackJunction(0, 3899, 1, 83, true)

local isValid, isActive = IsTrackJunctionActive(junctionId)
if isValid then
	print(('Junction %i is %s'):format(junctionId, isActive and 'active' or 'inactive'))
else
	print('The junctionId is invalid')
end
```

## Parameters
* **junctionId**: The track junction handle
* **isActive**: Whether the track junction is active

## Return value
Returns true if junction id is valid, false otherwise.
