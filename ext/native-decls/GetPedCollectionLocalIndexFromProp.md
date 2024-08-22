---
ns: CFX
apiset: client
game: gta5
---
## GET_PED_COLLECTION_LOCAL_INDEX_FROM_PROP

```c
int GET_PED_COLLECTION_LOCAL_INDEX_FROM_PROP(Ped ped, int anchorPoint, int propIndex);
```

Gets local index inside a collection (which can be obtained using [GET_PED_COLLECTION_NAME_FROM_PROP](#_0x8ED0C17)) for the given global prop index. The collection name and index are used in functions like [SET_PED_COLLECTION_PROP_INDEX](#_0x75240BCB).

## Examples

```lua
local ped = PlayerPedId()
-- Hat for mp_f_freemode_01. From female_freemode_beach collection under index 1.
-- Global index is 21 because there is 20 head prop variations in the base game collection that goes before the female_freemode_beach collection.
local name = GetPedPropCollectionName(ped, 0, 21)
local index = GetPedPropCollectionLocalIndex(ped, 0, 21)
-- Equivalent to SetPedPropIndex(ped, 0, 21, 0, false)
SetPedCollectionPropIndex(ped, 0, name, index, 0, false)
```

## Parameters
* **ped**: The target ped
* **anchorPoint**: One of the anchor points from [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F)
* **propIndex**: Global prop index. Same as set by `drawableId` in [SET_PED_PROP_INDEX](#_0x93376B65A266EB5F). Global prop index points to prop as if props from all collections for the given component are placed into one continuous array.

## Return value
Local index inside a collection that the given global prop index corresponds to. Returns -1 if Ped is not found or the global index is out of bounds.

