---
ns: CFX
apiset: server
---
## CREATE_TRAIN_CARRIAGE

```c
Vehicle CREATE_TRAIN_CARRIAGE(Vehicle train, Hash modelHash, float distanceFromEngine);
```

Creates and attaches train carriage to the target train.

## Parameters
* **train**: The target train.
* **modelHash**: The model of train to spawn.
* **distanceFromEngine**: how far the carriage should be from the engine carriage, maximum 1000.0f

## Return value
A script handle for the train carriage.

## Examples
```lua
--[[
    To work out distanceFromEngine:
    First Carriage:
    lengthOfCarriage = (modelBoundBoxMaxY - modelBoundBoxMinY)
    distanceFromEngine = lengthOfCarriage + carriageGap (found in trains.xml)

    Each Additional Carriage: add distance to previous carriage distanceFromEngine

    Example with metrotrain, trainconfigIndex 26 (b2802)
    lengthOfCarriage = (5.611063 - -5.667999) 
    distanceFromEngine = lengthOfCarriage + -0.5 = 10.7791
]]

-- Coordinates of the first node (0) in track 3
local coordinates = vec3(193.196, -603.836, 16.7565)
local metro = CreateTrain(`metrotrain`, coordinates, true, true, 12.0, 3, 25)

-- This carriage would be flipped as in trains.xml 'flip_model_dir' is true for the second carriage 
local metroCarriage = CreateTrainCarriage(metro, `metrotrain`, 10.7791)
print(GetTrainCarriageIndex(metroCarriage)) -- Will return 1. 
```