---
ns: CFX
apiset: client
game: gta5
---
## GET_NUMBER_OF_PED_COLLECTION_TEXTURE_VARIATIONS

```c
int GET_NUMBER_OF_PED_COLLECTION_TEXTURE_VARIATIONS(Ped ped, int componentId, char* collection, int drawableId);
```

An alternative to [GET_NUMBER_OF_PED_TEXTURE_VARIATIONS](#_0x8F7156A3142A6BAD) that uses local collection indexing instead of the global one.

## Parameters
* **ped**: The target ped
* **componentId**: One of the components from [SET_PED_COMPONENT_VARIATION](#_0x262B14F48D29DE80)
* **collection**: Name of the collection. Empty string for the base game collection. See [GET_PED_COLLECTION_NAME](#_0xFED5D83A) in order to list all available collections.
* **drawableId**: Local drawable Id inside the given collection. Refer to [GET_NUMBER_OF_PED_COLLECTION_DRAWABLE_VARIATIONS](#_0x310D0271).

## Return value
Number of texture variations available for the given drawable component. Returns 0 if ped or collection does not exist or index is out of bounds.
