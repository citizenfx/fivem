---
ns: CFX
apiset: client
---
## SOCKET_DISCONNECT

```c
BOOL SOCKET_DISCONNECT(int socketId);
```

Disconnects a WebSocket connection with the given socket ID.

This function closes the WebSocket connection gracefully and cleans up all associated resources, including any registered event handlers.

## Parameters
* **socketId**: The socket ID returned by `SOCKET_CONNECT`

## Return value
Returns `true` if the disconnection was successful, `false` if the socket ID was not found.

## Examples

```lua
-- Connect and then disconnect
local socketId = SocketConnect("ws://localhost:8080", function(eventType, data)
    print("Event: " .. eventType)
end)

-- Later, disconnect
if SocketDisconnect(socketId) then
    print("Disconnected successfully")
else
    print("Socket not found or already disconnected")
end
```

```js
// JavaScript example
const socketId = SocketConnect("ws://localhost:8080", (eventType, data) => {
    console.log(`Event: ${eventType}`);
});

// Later, disconnect
if (SocketDisconnect(socketId)) {
    console.log("Disconnected successfully");
}
```

```cs
// C# example
int socketId = Function.Call<int>(Hash.SOCKET_CONNECT, "ws://localhost:8080", new Action<string, string>((eventType, data) => { }));

// Later, disconnect
bool success = Function.Call<bool>(Hash.SOCKET_DISCONNECT, socketId);
```
