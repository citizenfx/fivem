---
ns: CFX
apiset: server
---
## GET_PED_SCRIPT_TASK_COMMAND

```c
Hash GET_PED_SCRIPT_TASK_COMMAND(Ped ped);
```

Gets the script task command currently assigned to the ped.

## Parameters
* **ped**: The target ped.

## Return value
The script task command currently assigned to the ped. A value of 0x811E343C denotes no script task is assigned.
