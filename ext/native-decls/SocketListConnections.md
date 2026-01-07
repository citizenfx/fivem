---
ns: CFX
apiset: client
---
## SOCKET_LIST_CONNECTIONS

```c
object SOCKET_LIST_CONNECTIONS();
```

Returns a list of all active WebSocket connections.

Each connection in the returned list includes:
- `id`: The socket ID
- `url`: The WebSocket URL
- `state`: The current connection state ("connecting", "open", "closing", "closed", "error")

## Return value
An array of objects containing information about each active connection.

The returned data follows this structure:
```
[
    {
        "id": 1,
        "url": "ws://localhost:8080",
        "state": "open"
    },
    {
        "id": 2,
        "url": "ws://example.com/socket",
        "state": "connecting"
    }
]
```

## Examples

```lua
-- List all active connections
local connections = SocketListConnections()
print("Active connections: " .. #connections)

for _, conn in ipairs(connections) do
    print(string.format("Socket %d: %s [%s]", conn.id, conn.url, conn.state))
end
```

```js
// JavaScript example
const connections = SocketListConnections();
console.log(`Active connections: ${connections.length}`);

connections.forEach(conn => {
    console.log(`Socket ${conn.id}: ${conn.url} [${conn.state}]`);
});
```

```cs
// C# example
dynamic connections = Function.Call<object>(Hash.SOCKET_LIST_CONNECTIONS);
foreach (var conn in connections)
{
    Debug.WriteLine($"Socket {conn.id}: {conn.url} [{conn.state}]");
}
```
