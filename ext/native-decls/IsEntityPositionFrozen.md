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


## Parameters
* **entity**: The entity to check for

## Return value
Returns `true` if the entity is frozen

## Examples

```lua
local isFrozen = IsEntityPositionFrozen(PlayerPedId())
```

```js
const isFrozen = IsEntityPositionFrozen(PlayerPedId());
```
