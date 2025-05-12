---
ns: CFX
apiset: server
game: gta5
---
## GET_TRAIN_FORWARD_CARRIAGE

```c
int GET_TRAIN_FORWARD_CARRIAGE(Vehicle train);
``` 

## Parameters
* **train**: The train handle

## Return value
The handle of the carriage in front of this train in the chain. Otherwise returns 0 if the train has no carriage in front of it 