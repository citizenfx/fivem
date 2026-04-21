---
ns: CFX
apiset: client
game: gta5
---
## GET_WEAPON_COMPONENT_RANGE_DAMAGE_MODIFIER

```c
float GET_WEAPON_COMPONENT_RANGE_DAMAGE_MODIFIER(Hash componentHash);
```

A getter for `CWeaponFallOffModifier`'s `DamageModifier` value on a weapon component.

```xml
<FallOffModifier type="CWeaponFallOffModifier">
	<DamageModifier value="1.333333" />
</FallOffModifier>
```

`1.0` would be the same damage, `0.5` would be half the damage, `1.25` would give you `25%` more damage

## Parameters
* **componentHash**: Weapon component name hash.

## Return value
Returns the weapons components damage fall off modifier.
