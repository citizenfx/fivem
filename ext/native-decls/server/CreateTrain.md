---
ns: CFX
apiset: server
---
## CREATE_TRAIN

```c
Vehicle CREATE_TRAIN(Hash modelHash, float x, float y, float z, bool direction, bool stopsAtStations, float cruiseSpeed, int trackId, int trainConfigIndex);
```

Creates a functioning train using 'server setter' logic (like CREATE_VEHICLE_SERVER_SETTER and CREATE_AUTOMOBILE).

## Parameters
* **modelHash**: The model of train to spawn.
* **x**: Spawn coordinate X component.
* **y**: Spawn coordinate Y component.
* **z**: Spawn coordinate Z component.
* **direction**: The direction in which the train will go (true = forward, false = backward)
* **stopsAtStations**: Should the train stop at stations on the track (>b2372)
* **cruiseSpeed**: The desired speed for the train to achieve (maximum 30.0f)
* **trackId**: The track the train should follow (0 = Main Track, 3 = Metro track)
* **trainConfigIndex**: The train variation id, found in trains.xml

## Return value
A script handle for the train.

## Examples
```lua
-- Coordinates of the first node (0) in track 3
local coordinates = vec3(193.196, -603.836, 16.7565)
local metro = CreateTrain(`metrotrain`, coordinates, true, true, 12.0, 3, 25)
print(GetTrainCarriageIndex(metro)) -- should return 0
```