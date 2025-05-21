---
ns: CFX
apiset: client
game: gta5
---
## GET_TRACK_JUNCTION_INFO

```c
bool GET_TRACK_JUNCTION_INFO(int junctionId, int* trackIndex, int* trackNode, int* newIndex, int* newNode, bool* direction);
```

## Examples

```lua
local onTrack = 0
local onNode = 3899
local newTrack = 1
local newNode = 83
local direction = true

local junctionId = RegisterTrackJunction(onTrack, onNode, newTrack, newNode, direction)
print(("The junctionId is %s"):format(junctionId))

local isValid, _onTrack, _onNode, _newTrack, _newNode, _direction = GetTrackJunctionInfo(junctionId)
if isValid then
	print(('The junction is valid, on track %i, on node %i, new track %i, new node %i, direction %s'):format(_onTrack, _onNode, _newTrack, _newNode, _direction and 'true' or 'false'))
else
	print('The junctionId is invalid')
end
```

## Parameters
* **junctionId**: The track junction handle
* **trackIndex**: The track index a train should be on
* **trackNode**: The node a train should be on
* **newIndex**: The new track index for a train to be placed on
* **newNode**: The new track node for a train to be placed on
* **direction**: The direction a train should be traveling for this junction

## Return value
Returns true if junction id is valid, false otherwise.