---
ns: CFX
apiset: client
game: gta5
---
## UNREGISTER_KEY_MAPPING

```c
void UNREGISTER_KEY_MAPPING(char* commandString);
```

Unregisters a key mapping for the current resource that was previously registered with [REGISTER_KEY_MAPPING](#_0xD7664FD1).

The key mapping will be removed from the settings menu and the associated binding will be unmapped. This only affects key mappings registered by the calling resource.

If the command has an alternate key binding (prefixed with `~!`), it will also be removed automatically.

## Parameters
* **commandString**: The command that was used when registering the key mapping, e.g. `+handsup`.

## Examples

```lua
RegisterKeyMapping('+handsup', 'Hands Up', 'keyboard', 'i')
RegisterKeyMapping('~!+handsup', 'Hands Up - Alternate Key', 'keyboard', 'o')

UnregisterKeyMapping('+handsup')
```

```js
RegisterKeyMapping('+handsup', 'Hands Up', 'keyboard', 'i');
RegisterKeyMapping('~!+handsup', 'Hands Up - Alternate Key', 'keyboard', 'o');

UnregisterKeyMapping('+handsup');
```
