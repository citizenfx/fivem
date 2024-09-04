---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_DRAWABLE_GLOBAL_INDEX_FROM_COLLECTION

```c
int GET_PED_DRAWABLE_GLOBAL_INDEX_FROM_COLLECTION(Ped ped, int componentId, char* collection, int drawableId);
```

Returns global drawable index based on the local one. Is it a reverse to [GET_PED_COLLECTION_NAME_FROM_DRAWABLE](#_0xD6BBA48B) and [GET_PED_COLLECTION_LOCAL_INDEX_FROM_DRAWABLE](#_0x94EB1FE4) natives.

Drawables are stored inside collections. Each collection usually corresponds to a certain DCL or the base game.

If all drawables from all collections are placed into one continuous array - the global index will correspond to the index of drawable in such array. Local index is index of drawable in this array relative to the start of the given collection.

## Parameters
* **ped**: The target ped
* **componentId**: One of the components from [SET_PED_COMPONENT_VARIATION](#_0x262B14F48D29DE80)
* **collection**: Name of the collection. Empty string for the base game collection. See [GET_PED_COLLECTION_NAME](#_0xFED5D83A) in order to list all available collections.
* **drawableId**: Local drawable Id inside the given collection. Refer to [GET_NUMBER_OF_PED_COLLECTION_DRAWABLE_VARIATIONS](#_0x310D0271).

## Return value
Global drawable ID that corresponds to the given collection and local drawable index. Returns -1 if Ped or collection does not exist or local index is out of bounds.
