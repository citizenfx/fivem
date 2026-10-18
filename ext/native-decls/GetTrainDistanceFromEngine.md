---
ns: CFX
apiset: server
game: gta5
---
## GET_TRAIN_DISTANCE_FROM_ENGINE

```c
float GET_TRAIN_DISTANCE_FROM_ENGINE(Vehicle train);
```

Gets how far along the chain this carriage sits, measured from the engine carriage. Returns 0.0 for the engine itself.

## Parameters
* **train**: The train handle

## Return value
The distance from the engine carriage, or 0.0 if the entity is not a train.
