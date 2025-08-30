---
ns: CFX
apiset: client
game: rdr3
---

## GET_PED_BONE_MATRIX

```c
void GET_PED_BONE_MATRIX(Ped ped, int boneId, Vector3* forwardVector, Vector3* rightVector, Vector3* upVector, Vector3* position);
```

Returns the bone matrix of the specified bone id. usefull for entity attachment

## Examples
```lua
local fowardVector, rightVector, upVector, position = GetPedBoneMatrix(PlayerPedId(),boneId)
```

## Parameters
* **ped**: 
* **boneId**: 


## Return value
* **forwardVector**: 
* **rightVector**: 
* **upVector**: 
* **position**: 

