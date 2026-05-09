---
ns: CFX
apiset: client
game: gta5
---
## GET_WEAPON_COMPONENT_DAMAGE_MODIFIER

```c
float GET_WEAPON_COMPONENT_DAMAGE_MODIFIER(Hash componentHash);
```

A getter for `CWeaponDamageModifier`'s `DamageModifier` value on a weapon component.

```xml
<DamageModifier type="CWeaponDamageModifier">
    <DamageModifier value="1.000000" />
</DamageModifier>
```

## Parameters
* **componentHash**: Weapon component name hash.

## Return value
Returns the weapon components damage modifier.
