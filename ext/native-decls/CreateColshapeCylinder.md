---
ns: CFX
apiset: client
---
## CREATE_COLSHAPE_CYLINDER

```c
BOOL CREATE_COLSHAPE_CYLINDER(string colShapeId, float x, float y, float z, float radius, float height, bool infinite = false);
```

## Examples

Creates a Colshape in the Game world. Whenever the clients PlayerPed is entering this shape, an event gets fired:

```lua
AddEventHandler("onPlayerEnterColShape", function(shapeId)
    print("Player entered collision shape: " .. shapeId)
    -- Additional logic based on the shapeId can be added here.
end)

-- On Exit, a seperate Event is getting fired:
AddEventHandler("onPlayerLeaveColShape", function(shapeId)
    print("Player left collision shape: " .. shapeId)
    -- Additional logic based on the shapeId can be added here.
end)
```

Colshapes trigger 10 times per second (so every 100ms) which should be more than sufficient for most cases.

The internal system that handles Collision detection splits up the game map into a Grid that is 1000x1000 each.
If you for some reason want to create a colshape that would be bigger than 1000x1000 it is recommended to use the
infinite parameter.
All Colshapes that have the infinite parameter set to true will get checked no matter your position on the
world grid. All other Colshapes will get checked on the 1000x1000 approach. This means that the system will not check for
Shapes that are created on the North side of the map if you are in the south to save resources.

## Parameters
* **colShapeId**: A unique identifier for the collision shape.
* **x**: X world coordinate of the center of the cylinder.
* **y**: Y world coordinate of the center of the cylinder.
* **z**: Z world coordinate of the center of the cylinder.
* **radius**: Radius of the cylinder.
* **height**: Height of the cylinder.
* **infinite**: Optional. If true, this shape will get checked no matter your position on the map. Mandatory for large Colshapes (larger than 1000x1000).

## Return value
Returns true, if the Colshape has been successfully created. False on any errors or if the ID is taken.
