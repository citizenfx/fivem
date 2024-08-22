---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_COLLECTION_NAME_FROM_DRAWABLE

```c
char* GET_PED_COLLECTION_NAME_FROM_DRAWABLE(Ped ped, int componentId, int drawableId);
```

Gets collection name for the given global drawable ID. Together with [GET_PED_COLLECTION_LOCAL_INDEX_FROM_DRAWABLE](#_0x94EB1FE4) is used to get collection and local index (inside the given collection) of the drawable. The collection name and index are used in functions like [SET_PED_COLLECTION_COMPONENT_VARIATION](#_0x88711BBA).

## Examples

```lua
local ped = PlayerPedId()
-- Top for mp_f_freemode_01. From female_freemode_beach collection under index 1.
-- Global index is 17 because there is 16 top variations in the base game collection that goes before the female_freemode_beach collection.
local name = GetPedDrawableCollectionName(ped, 11, 17)
local index = GetPedDrawableCollectionLocalIndex(ped, 11, 17)
-- Equivalent to SetPedComponentVariation(ped, 11, 17, 0, 0)
SetPedCollectionComponentVariation(ped, 11, name, index, 0, 0)
```

## Parameters
* **ped**: The target ped
* **componentId**: One of the components from [SET_PED_COMPONENT_VARIATION](#_0x262B14F48D29DE80)
* **drawableId**: Global drawable ID. Same as set in [SET_PED_COMPONENT_VARIATION](#_0x262B14F48D29DE80). Global drawable ID points to drawables as if drawables from all collections for the given component are placed into one continuous array.

## Return value
Name of the collection that the given global drawable ID corresponds to. Base game collection is an empty string. Returns null if Ped is not found or the global index is out of bounds.
