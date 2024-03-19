---
ns: CFX
apiset: shared
game: gta5
---
## IS_ENTITY_POSITION_FROZEN

```c
bool IS_ENTITY_POSITION_FROZEN(Entity entity);
```
A getter for [FREEZE_ENTITY_POSITION](#_0x428CA6DBD1094446).

## Examples

```lua
local isFrozen = IsEntityPositionFrozen(PlayerPedId())
```

```js
const isFrozen = IsEntityPositionFrozen(PlayerPedId());
```

## Parameters
* **entity**: The entity to check for

## Return value
Boolean stating if it is frozen or not.