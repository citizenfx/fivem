---
ns: CFX
apiset: client
game: gta5
---
## GET_TRAIN_DOOR_OPEN_RATIO

```c
float GET_TRAIN_DOOR_OPEN_RATIO(Vehicle train, int doorIndex);
```

Gets the ratio that a door is open for on a train.

## Parameters
* **train**: The train to obtain the door ratio for.
* **doorIndex**: Zero-based door index.

## Return value
A value between 0.0 (fully closed) and 1.0 (fully open).

## Examples

```lua
local doorCount = GetTrainDoorCount(train)
for doorIndex = 0, doorCount - 1 do
    local ratio = GetTrainDoorOpenRatio(train, doorIndex)
    print("Door " .. tostring(doorIndex) .. " is open by a ratio of " .. tostring(ratio))
end
```

```cs
int doorCount = API.GetTrainDoorCount(train);
for (int doorIndex = 0; doorIndex < doorCount; doorIndex++)
{
    float ratio = API.GetTrainDoorOpenRatio(train, doorIndex);
    Debug.WriteLine($"Door {doorIndex} is open by a ratio of {ratio}");
}
```