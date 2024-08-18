---
ns: CFX
apiset: client
game: gta5
---
## SET_TRAIN_DOOR_OPEN_RATIO

```c
void SET_TRAIN_DOOR_OPEN_RATIO(Vehicle train, int doorIndex, float ratio);
```

Sets the ratio that a door is open for on a train.

## Parameters
* **train**: The train to set the door ratio for.
* **doorIndex**: Zero-based door index.
* **ratio**: A value between 0.0 (fully closed) and 1.0 (fully open).

## Examples

```lua
-- open all doors on a train
local doorCount = GetTrainDoorCount(train)
for doorIndex = 0, doorCount - 1 do
    SetTrainDoorOpenRatio(train, doorIndex, 1.0)
end
```

```cs
// open all doors on a train
int doorCount = API.GetTrainDoorCount(train);
for (int doorIndex = 0; doorIndex < doorCount; doorIndex++)
{
    API.SetTrainDoorOpenRatio(train, doorIndex, 1.0f);
}
```