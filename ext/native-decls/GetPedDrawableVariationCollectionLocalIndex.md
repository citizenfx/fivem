---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_DRAWABLE_VARIATION_COLLECTION_LOCAL_INDEX

```c
int GET_PED_DRAWABLE_VARIATION_COLLECTION_LOCAL_INDEX(Ped ped, int componentId);
```

An analogue to [GET_PED_DRAWABLE_VARIATION](#_0x67F3780DD425D4FC) that returns collection local drawable index (inside [GET_PED_DRAWABLE_VARIATION_COLLECTION_NAME](#_0xBCE0AB63) collection) instead of the global drawable index.

## Parameters
* **ped**: The target ped
* **componentId**: One of the components from [SET_PED_COMPONENT_VARIATION](#_0x262B14F48D29DE80)

## Return value
Local drawable index of the drawable that is currently used in the given ped and component.
