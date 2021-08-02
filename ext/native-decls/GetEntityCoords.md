---
ns: CFX
apiset: server
---

## GET_ENTITY_COORDS

```c
Vector3 GET_ENTITY_COORDS(Entity entity);
```

Gets the current coordinates for a specified entity. This native is used server side when using OneSync.

See [GET_ENTITY_COORDS](#_0x3FEF770D40960D5A) for client side.

## Parameters

- **entity**: The entity to get the coordinates from.

## Return value

The current entity coordinates.

## Examples

```lua
local function ShowCoordinates()
    local player = source
    local ped = GetPlayerPed(player)
    local playerCoords = GetEntityCoords(ped)

    print(playerCoords) -- vector3(...)
end

RegisterNetEvent("myCoordinates")
AddEventHandler("myCoordinates", ShowCoordinates)
```

```js
onNet('myCoordinates', () => {
  const player = global.source; // use (global as any).source for Typescript
  const ped = GetPlayerPed(player);
  const [playerX, playerY, playerZ] = GetEntityCoords(ped);

  console.log(`${playerX}, ${playerY}, ${playerZ}`);
});
```

```cs
using static CitizenFX.Core.Native.API;
// ...

// In class constructor
EventHandlers["myCoordinates"] += new Action<Player>(ShowCoordinates);

// Delegate method
private void ShowCoordinates([FromSource]Player player) {
    Vector3 playerCoords = GetEntityCoords(player.Character);

    // or the preferred use of C# wrapper
    Vector3 playerCoords = player.Character.Position;

    Debug.WriteLine($"{playerCoords}");
}

```
