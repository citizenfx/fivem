---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_DRAWABLE_VARIATION_COLLECTION_NAME

```c
char* GET_PED_DRAWABLE_VARIATION_COLLECTION_NAME(Ped ped, int componentId);
```

An analogue to [GET_PED_DRAWABLE_VARIATION](#_0x67F3780DD425D4FC) that returns collection name instead of the global drawable index.

Should be used together with [GET_PED_DRAWABLE_VARIATION_COLLECTION_LOCAL_INDEX](#_0x9970386F).

## Parameters
* **ped**: The target ped
* **componentId**: One of the components from [SET_PED_COMPONENT_VARIATION](#_0x262B14F48D29DE80)

## Return value
Collection name to which the current drawable used in the given ped and component belongs to. Returns null if Ped is not found or index is out of bounds.
