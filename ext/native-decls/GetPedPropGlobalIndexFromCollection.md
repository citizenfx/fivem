---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_PROP_GLOBAL_INDEX_FROM_COLLECTION

```c
int GET_PED_PROP_GLOBAL_INDEX_FROM_COLLECTION(Ped ped, int anchorPoint, char* collection, int propIndex);
```

Returns global prop index based on the local one. Is it a reverse to [GET_PED_COLLECTION_NAME_FROM_PROP](#_0x8ED0C17) and [GET_PED_COLLECTION_LOCAL_INDEX_FROM_PROP](#_0xFBDB885F) natives.

Props are stored inside collections. Each collection usually corresponds to a certain DCL or the base game.

If all props from all collections are placed into one continuous array - the global index will correspond to the index of the prop in such array. Local index is index of the prop in this array relative to the start of the given collection.

## Parameters
* **ped**: The target ped
* **anchorPoint**: One of the anchor points from [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F)
* **collection**: Name of the collection. Empty string for the base game collection. See [GET_PED_COLLECTION_NAME](#_0xFED5D83A) in order to list all available collections.
* **propIndex**: Local prop index inside the given collection. Refer to [GET_NUMBER_OF_PED_COLLECTION_PROP_DRAWABLE_VARIATIONS](#_0x3B6A13E1).

## Return value
Global prop index that corresponds to the given collection and local prop index. Returns -1 if Ped or collection does not exist or local index is out of bounds.
