---
ns: CFX
apiset: server
---
## GET_PED_SPECIFIC_TASK_TYPE

```c
int GET_PED_SPECIFIC_TASK_TYPE(Ped ped, int index);
```

Gets the type of a ped's specific task given an index of the CPedTaskSpecificDataNode nodes.
A ped will typically have a task at index 0, if a ped has multiple tasks at once they will be in the order 0, 1, 2, etc.

## Parameters
* **ped**: The target ped.
* **index**: A zero-based index with a maximum value of 7.

## Return value
The type of the specific task.
1604: A value of 530 denotes no script task is assigned or an invalid input was given.
2060+: A value of 531 denotes no script task is assigned or an invalid input was given.