---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_PROP_COLLECTION_NAME

```c
char* GET_PED_PROP_COLLECTION_NAME(Ped ped, int anchorPoint);
```

An analogue to [GET_PED_PROP_INDEX](#_0x898CC20EA75BACD8) that returns collection name instead of the global drawable index.

Should be used together with [GET_PED_PROP_COLLECTION_LOCAL_INDEX](#_0xCD420AD1).

## Parameters
* **ped**: The target ped
* **anchorPoint**: One of the anchor points from [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F)

## Return value
Collection name to which the current prop used in the given ped and anchor point belongs to. Returns null if Ped is not found, does not have a prop at the specified anchor point, or if the index is out of bounds.
