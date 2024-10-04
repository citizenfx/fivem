---
ns: CFX
apiset: client
game: gta5
---
## GET_NUMBER_OF_PED_COLLECTION_DRAWABLE_VARIATIONS

```c
int GET_NUMBER_OF_PED_COLLECTION_DRAWABLE_VARIATIONS(Ped ped, int componentId, char* collection);
```

An analogue of [GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS](#_0x27561561732A7842) that returns number of drawable variations inside a single collection instead of the total number across all collections.

## Parameters
* **ped**: The target ped
* **componentId**: One of the components from [SET_PED_COMPONENT_VARIATION](#_0x262B14F48D29DE80)
* **collection**: Name of the collection. Empty string for the base game collection. See [GET_PED_COLLECTION_NAME](#_0xFED5D83A) in order to list all available collections.

## Return value
Number of drawables available in the given collection. Returns 0 if ped or collection does not exist.
