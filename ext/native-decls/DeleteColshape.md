---
ns: CFX
apiset: client
---
## CREATE_COLSHAPE_CIRCLE

```c
BOOL DELETE_COLSHAPE(string colShapeId);
```

## Examples

Creates a Colshape in the Game world. Whenever the clients PlayerPed is entering this shape, an event gets fired:

```lua

local created = CreateColshapeCircle("circleshape1", 500.0, 500.0, 100.0, 20.0)
-- now if we delete the shape:
DeleteColshape("circleshape1")

-- none of the events will get triggered for this shapeId
AddEventHandler("onPlayerEnterColShape", function(shapeId)
    if shapeId == "circleshape1" then
        print("entered circleshape1")
    end
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
* **colShapeId**: A unique identifier for the Colshape that you want to delete.

## Return value
Returns true if the Colshape has been successfully deleted, false on any errors or if the ID does not exist.
