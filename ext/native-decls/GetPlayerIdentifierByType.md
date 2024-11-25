---
ns: CFX
apiset: server
---
## GET_PLAYER_IDENTIFIER_BY_TYPE

```c
char* GET_PLAYER_IDENTIFIER_BY_TYPE(char* playerSrc, char* identifierType);
```

Get an identifier from a player by the type of the identifier.
Known [Identifiers](https://docs.fivem.net/docs/scripting-reference/runtimes/lua/functions/GetPlayerIdentifiers/#identifier-types)

## Parameters

- **playerSrc**: The player to get the identifier for
- **identifierType**: The string to match in an identifier, this can be `"license"` for example.

## Return value

The identifier that matches the string provided

## Examples

```lua
local playerLicenses = {}

AddEventHandler('playerJoining', function()
    playerLicenses[source] = GetPlayerIdentifierByType(source, 'license')
end)
```

```js
let playerLicenses = {};

on('playerJoining', () => {
    playerLicenses[source] = GetPlayerIdentifierByType(source, 'license');
});
```

```cs
using System.Collections.Generic;
using static CitizenFX.Core.Native.API;
// ...

// In class
private Dictionary<int, string> PlayerLicenses = new Dictionary<int, string>();

// In class constructor
EventHandlers["playerJoining"] += new Action<Player>(SetLicense);

// Delegate method
private void SetLicense([FromSource]Player player) {
    PlayerLicenses.Add(player.Handle, GetPlayerIdentifierByType(player.Handle, "license"));
}
```
