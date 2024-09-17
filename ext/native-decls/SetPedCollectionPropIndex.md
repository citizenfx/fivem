---
ns: CFX
apiset: client
game: gta5
---
## SET_PED_COLLECTION_PROP_INDEX

```c
void SET_PED_COLLECTION_PROP_INDEX(Ped ped, int anchorPoint, char* collection, int propIndex, int textureId, BOOL attach);
```

An alternative to [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F) that uses local collection indexing instead of the global one.

The local / collection relative indexing is useful because the global index may get shifted after Title Update. While local index will remain the same which simplifies migration to the newer game version.

Collection name and local index inside the collection can be obtained from the global index using [GET_PED_COLLECTION_NAME_FROM_PROP](#_0x8ED0C17) and [GET_PED_COLLECTION_LOCAL_INDEX_FROM_PROP](#_0xFBDB885F) natives.

## Parameters
* **ped**: The target ped
* **anchorPoint**: One of the anchor points from [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F)
* **collection**: Name of the collection. Empty string for the base game collection. See [GET_PED_COLLECTION_NAME](#_0xFED5D83A) in order to list all available collections.
* **propIndex**: Local prop index inside the given collection. Refer to [GET_NUMBER_OF_PED_COLLECTION_PROP_DRAWABLE_VARIATIONS](#_0x3B6A13E1).
* **textureId**: The texture id of the drawable. Refer to [GET_NUMBER_OF_PED_COLLECTION_PROP_TEXTURE_VARIATIONS](#_0x75CAF9CC).
* **attach**: Attached or not.
