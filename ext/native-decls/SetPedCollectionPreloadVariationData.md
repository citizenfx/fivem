---
ns: CFX
apiset: client
game: gta5
---
## SET_PED_COLLECTION_PRELOAD_VARIATION_DATA

```c
void SET_PED_COLLECTION_PRELOAD_VARIATION_DATA(Ped ped, int componentId, char* collection, int drawableId, int textureId);
```

An alternative to [SET_PED_PRELOAD_VARIATION_DATA](#_0x39D55A620FCB6A3A) that uses local collection indexing instead of the global one.

The local / collection relative indexing is useful because the global index may get shifted after Title Update. While local index will remain the same which simplifies migration to the newer game version.

Collection name and local index inside the collection can be obtained from the global index using [GET_PED_COLLECTION_NAME_FROM_DRAWABLE](#_0xD6BBA48B) and [GET_PED_COLLECTION_LOCAL_INDEX_FROM_DRAWABLE](#_0x94EB1FE4) natives.

## Parameters
* **ped**: The target ped
* **componentId**: One of the components from [SET_PED_COMPONENT_VARIATION](#_0x262B14F48D29DE80)
* **collection**: Name of the collection. Empty string for the base game collection. See [GET_PED_COLLECTION_NAME](#_0xFED5D83A) in order to list all available collections.
* **drawableId**: Local drawable Id inside the given collection. Refer to [GET_NUMBER_OF_PED_COLLECTION_DRAWABLE_VARIATIONS](#_0x310D0271).
* **textureId**: The texture id of the drawable. Refer to [GET_NUMBER_OF_PED_COLLECTION_TEXTURE_VARIATIONS](#_0xD2C15D7).
