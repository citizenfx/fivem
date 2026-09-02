---
ns: CFX
apiset: server
game: gta5
---
## GET_TRAIN_CONFIG_INDEX

```c
int GET_TRAIN_CONFIG_INDEX(Vehicle train);
```

Gets the index of the train configuration this train was created from, as defined in `traintracks.xml`.

## Parameters
* **train**: The train handle

## Return value
The train's config index, or -1 if the entity is not a train.
