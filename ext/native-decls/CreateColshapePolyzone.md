---
ns: CFX
apiset: client
game: gta5
---
## CREATE_COLSHAPE_POLYZONE
```c
bool CREATE_COLSHAPE_POLYZONE(char* colShapeId, char* polyZoneData, int dataLength, float minZ, float maxZ);
```

Creates a polyzone collision shape defined by a set of vertices along with vertical boundaries.  
The `polyZoneData` parameter must contain a msgpack‑encoded array of vertices to define the polygon. Accepted vertex formats include vector2, vector3, and vector4 (only the x and y components are used). Normal tables with keys (e.g. `{ x = 2.0, y = 2.0 }`) and arrays (e.g. `{ [1] = 1.0, [2] = 2.0 }`) are also supported.

If parameters 4 (`minZ`) and 5 (`maxZ`) are empty, the system assumes very large numbers (effectively infinite), so basically we ignore Z.

When a player enters or leaves the collision shape, the game fires the `onPlayerEnterColshape` or `onPlayerLeaveColshape` event with the provided `colShapeId`.

### Parameters
* **colShapeId**: The unique identifier for the collision shape.
* **polyZoneData**: A pointer to a msgpack‑encoded buffer containing an array of vertices (vector2, vector3, or vector4) that define the polygon.
* **dataLength**: The length of the msgpack‑encoded buffer.
* **minZ**: The explicit minimum Z coordinate for the collision shape. If omitted, an extremely low value is assumed (effectively negative infinity).
* **maxZ**: The explicit maximum Z coordinate for the collision shape. If omitted, an extremely high value is assumed (effectively positive infinity).

### Return Value
Returns `true` if the collision shape was created successfully, or `false` if the identifier is already taken or an error occurred while processing the data.

### Examples
In this example, a 2D polygon is defined by four vertices and explicit vertical boundaries are provided:

```lua
-- Define a simple rectangular polyzone using 2D vertices
local vertices = {
    { x = 100.0, y = 100.0 },
    { x = 200.0, y = 100.0 },
    { x = 200.0, y = 200.0 },
    { x = 100.0, y = 200.0 }
}
-- You can also use vector2!
local packedVertices = msgpack.pack(vertices)
-- Create the polyzone with explicit minZ and maxZ (e.g. 50 and 150)
local success = CREATE_COLSHAPE_POLYZONE("myPolyzone", packedVertices, packedVertices:len(), 50.0, 150.0)
if success then
    print("Polyzone created successfully!")
else
    print("Failed to create polyzone.")
end
```

In this second example, vertices include a Z coordinate. Passing empty vertical boundaries (e.g., nil) will assume infinite values:

```lua
-- Define a polyzone with 3D vertices (only x and y are used)
local vertices3D = {
    { x = 100.0, y = 100.0, z = 50.0 },
    { x = 200.0, y = 100.0, z = 55.0 },
    { x = 200.0, y = 200.0, z = 60.0 },
    { x = 100.0, y = 200.0, z = 50.0 }
}
-- You can also use vector3 or vector4 types.
local packedVertices3D = msgpack.pack(vertices3D)
-- Passing nil for minZ and maxZ assumes infinite vertical boundaries.
local success = CREATE_COLSHAPE_POLYZONE("myPolyzone3D", packedVertices3D, packedVertices3D:len(), nil, nil)
if success then
    print("Polyzone created successfully with infinite vertical boundaries!")
else
    print("Failed to create polyzone.")
end
```
