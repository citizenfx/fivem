---
ns: CFX
apiset: client
---
## MUMBLE_SET_SERVER_ADDRESS

```c
void MUMBLE_SET_SERVER_ADDRESS(char* address, int port);
```

Changes the Mumble server address to connect to, and reconnects to the new address.

Setting the address to an empty string and the port to -1 will reset to the built in FXServer Mumble Implementation.

## Parameters
* **address**: The address of the mumble server.
* **port**: The port of the mumble server.
