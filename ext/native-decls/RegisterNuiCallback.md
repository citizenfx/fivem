---
ns: CFX
apiset: client
---
## REGISTER_NUI_CALLBACK

```c
void REGISTER_NUI_CALLBACK(char* eventName, func callback);
```

**NOTE**: `eventName` is expected to be *unique*, calling this native twice with the same event name will result in the first one being overwrote.

## Parameters
* **eventName**: The event name to listen for
* **callback**: The function to callback to

## Examples
```js
const someEventCallback = (data, cb) => {

}
RegisterNuiCallback("someEvent", someEventCallback)
```
