---
ns: CFX
apiset: server
---
## LOAD_PLAYER_COMMERCE_DATA_EXT

```c
void LOAD_PLAYER_COMMERCE_DATA_EXT(char* playerSrc);
```

Requests the commerce data from Tebex for the specified player, including the owned SKUs.

Use [`IS_PLAYER_COMMERCE_INFO_LOADED_EXT`](#_0x1D14F4FE) to check if it has loaded.

This will not automatically update whenever a client purchases a package, if you want to fetch new purchases you will need to call this native again.

This native will temporarily cache the players commerce data for 10 seconds, a call to this native after 10 seconds will re-fetch the players commerce data.

## Parameters
* **playerSrc**: The player handle

## Examples
```lua
RegisterNetEvent("doesOwnPackage", function(packageIdSku)
	-- source isn't valid across waits, so we localize it
	local source = source

	-- input isn't right
	if type(packageIdSku) ~= "number" then
		return
	end

	-- The native will cache the results
	LoadPlayerCommerceDataExt(source)
	-- Wait for the players data to load
	while not IsPlayerCommerceInfoLoadedExt(source) do
		Wait(0)
	end

	-- Tell the client if they own the package or not
	TriggerClientEvent("doesOwnPackage", source, DoesPlayerOwnSkuExt(source, packageIdSku))
end)
```
