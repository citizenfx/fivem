---
ns: CFX
apiset: client
game: gta5
---
## GET_NUMBER_OF_PED_COLLECTION_PROP_DRAWABLE_VARIATIONS

```c
int GET_NUMBER_OF_PED_COLLECTION_PROP_DRAWABLE_VARIATIONS(Ped ped, int anchorPoint, char* collection);
```

An analogue of [GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS](#_0x5FAF9754E789FB47) that returns number of prop variations inside a single collection instead of the total number across all collections.

## Parameters
* **ped**: The target ped
* **anchorPoint**: One of the anchor points from [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F)
* **collection**: Name of the collection. Empty string for the base game collection. See [GET_PED_COLLECTION_NAME](#_0xFED5D83A) in order to list all available collections.

## Return value
Number of props available in the given collection. Returns 0 if ped or collection does not exist.
