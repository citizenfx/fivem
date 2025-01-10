---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_PROP_COLLECTION_LOCAL_INDEX

```c
int GET_PED_PROP_COLLECTION_LOCAL_INDEX(Ped ped, int anchorPoint);
```

An analogue to [GET_PED_PROP_INDEX](#_0x898CC20EA75BACD8) that returns collection local prop index (inside [GET_PED_PROP_COLLECTION_NAME](#_0x6B5653E4) collection) instead of the global prop index.

## Parameters
* **ped**: The target ped
* **anchorPoint**: One of the anchor points from [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F)

## Return value
Local drawable index of the drawable that is currently used in the given ped and component, or -1 if the ped does not have a prop at the specified anchor point
