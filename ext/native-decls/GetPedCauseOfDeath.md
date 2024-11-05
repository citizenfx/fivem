---
ns: CFX
apiset: server
---
## GET_PED_CAUSE_OF_DEATH

```c
Hash GET_PED_CAUSE_OF_DEATH(Ped ped);
```

You can use this along side [`GET_PED_SOURCE_OF_DAMAGE`](#_0x535DB43F) to get the killing entity.

## Parameters
* **ped**: The ped to get the death cause of

## Return value
Returns the hash of the weapon that caused the death of the player, or `0`
