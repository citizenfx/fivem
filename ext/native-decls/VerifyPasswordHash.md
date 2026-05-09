---
ns: CFX
apiset: server
---
## VERIFY_PASSWORD_HASH

```c
BOOL VERIFY_PASSWORD_HASH(char* password, char* hash);
```

**NOTE**: This cryptographic verification happens on the main thread, and should be used sparringly.

## Parameters
* **password**: The password to check
* **hash**: The hash it should equal, see [`GET_PASSWORD_HASH`](#_0x23473EA4)

## Return value
Returns `true` if `password` matched `hash`, `false` otherwise.

## Examples
```lua
local secretPassword = GetPasswordHash("superSecret!")
RegisterNetEvent("canPlayerAccess", function(password)
    if not VerifyPasswordHash(password, secretPassword) then
        TriggerClientEvent("playerCantAccess", source)
        return
    end

    TriggerClientEvent("playerCanAccess", source)
end)
