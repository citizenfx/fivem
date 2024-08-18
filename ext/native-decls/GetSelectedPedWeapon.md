---
ns: CFX
apiset: server
---
## GET_SELECTED_PED_WEAPON

```c
Hash GET_SELECTED_PED_WEAPON(Ped ped);
```

An alias of [GET_CURRENT_PED_WEAPON](#_0xB0237302).

Note, the client-side [GET_SELECTED_PED_WEAPON](#_0x0A6DB4965674D243) native returns the weapon selected via the HUD (weapon wheel). This data is not available to FXServer.

## Parameters
* **ped**: The target ped.

## Return value
The weapon hash.
