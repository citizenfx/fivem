---
ns: CFX
apiset: shared
game: gta5
---
## GET_TRAIN_STATE

```c
int GET_TRAIN_STATE(Vehicle train);
```

## Parameters
* **train**: The train handle

## Return value
The trains current state

```c
enum eTrainState
{
    MOVING = 0,
    ENTERING_STATION,
    OPENING_DOORS,
    STOPPED,
    CLOSING_DOORS,
    LEAVING_STATION,
}
```