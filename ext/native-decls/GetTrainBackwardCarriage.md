---
ns: CFX
apiset: server
game: gta5
---
## GET_TRAIN_BACKWARD_CARRIAGE

```c
int GET_TRAIN_BACKWARD_CARRIAGE(Vehicle train);
``` 

## Parameters
* **train**: The train handle

## Return value
The handle of the carriage behind this train in the chain. Otherwise returns 0 if the train is the caboose of the chain. 