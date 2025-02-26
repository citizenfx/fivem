---
ns: CFX
apiset: client
game: gta5
---
## GET_TRACK_JUNCTION_FROM_NODES

```c
int GET_TRACK_JUNCTION_FROM_NODES(int trackIndex, int trackNode, int newIndex, int newNode, bool direction);
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

local retrievedJunctionId = GetTrackJunctionFromNodes(onTrack, onNode, newTrack, newNode, direction)

if retrievedJunctionId ~= -1 then
	print(('The junction is valid, junctionId %i'):format(retrievedJunctionId))
else
	print('The junctionId is invalid')
end
```

## Parameters
* **trackIndex**: The track index a train should be on
* **trackNode**: The node a train should be on
* **newIndex**: The new track index for a train to be placed on
* **newNode**: The new track node for a train to be placed on
* **direction**: The direction a train should be traveling for this junction

## Return value
Returns the junction id for the given nodes, or -1 if no junction exists.