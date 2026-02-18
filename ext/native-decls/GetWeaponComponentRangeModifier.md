---
ns: CFX
apiset: client
game: gta5
---
## GET_WEAPON_COMPONENT_RANGE_MODIFIER

```c
float GET_WEAPON_COMPONENT_RANGE_MODIFIER(Hash componentHash);
```

A getter for `CWeaponFallOffModifier`'s `RangeModifier` value on a weapon component.

```xml
<FallOffModifier type="CWeaponFallOffModifier">
	<RangeModifier value="1.250000" />
</FallOffModifier>
```

`1.0` would be the same range, `0.5` would be half the range, `1.25` would give you `25%` more range before damage falls off

## Parameters
* **componentHash**: Weapon component name hash.

## Return value
Returns the weapon components range modifier
