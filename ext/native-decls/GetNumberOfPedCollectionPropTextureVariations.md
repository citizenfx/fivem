---
ns: CFX
apiset: client
game: gta5
---
## GET_NUMBER_OF_PED_COLLECTION_PROP_TEXTURE_VARIATIONS

```c
int GET_NUMBER_OF_PED_COLLECTION_PROP_TEXTURE_VARIATIONS(Ped ped, int anchorPoint, char* collection, int propIndex);
```

An alternative to [GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS](#_0xA6E7F1CEB523E171) that uses local collection indexing instead of the global one.

## Parameters
* **ped**: The target ped
* **anchorPoint**: One of the anchor points from [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F)
* **collection**: Name of the collection. Empty string for the base game collection. See [GET_PED_COLLECTION_NAME](#_0xFED5D83A) in order to list all available collections.
* **propIndex**: Local prop index inside the given collection. Refer to [GET_NUMBER_OF_PED_COLLECTION_PROP_DRAWABLE_VARIATIONS](#_0x3B6A13E1).

## Return value
Number of texture variations available for the given prop. Returns 0 if ped or collection does not exist or index is out of bounds.
