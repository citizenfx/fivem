---
ns: CFX
apiset: shared
---
## ADD_CONVAR_CHANGE_LISTENER

```c
int ADD_CONVAR_CHANGE_LISTENER(char* conVarFilter, func handler);
```

Adds a listener for Console Variable changes.

The function called expects to match the following signature:

```ts
function ConVarChangeListener(conVarName: string, reserved: any);
```

* **conVarName**: The ConVar that changed.
* **reserved**: Currently unused.

## Parameters
* **conVarFilter**: The Console Variable to listen for, this can be a pattern like "test:*", or null for any
* **handler**: The handler function.

## Return value
A cookie to remove the change handler.

## Examples
```js
// listen for all convar changes
AddConvarChangeListener(null, (conVarName, reserved) => {
    print(GetConvarInt(conVarName))
})

// listen to convars that start with "script:"
AddConvarChangeListener("script:*", (conVarName, reserved) => {
    print(GetConvarInt(conVarName))
})
```
