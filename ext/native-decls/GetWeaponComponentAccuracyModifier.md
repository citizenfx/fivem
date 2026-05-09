---
ns: CFX
apiset: client
game: gta5
---
## GET_WEAPON_COMPONENT_ACCURACY_MODIFIER

```c
float GET_WEAPON_COMPONENT_ACCURACY_MODIFIER(Hash componentHash);
```

A getter for `CWeaponAccuracyModifier` in a weapon component.

```xml
<AccuracyModifier type="CWeaponAccuracyModifier">
    <AccuracyModifier value="1.100000" />
</AccuracyModifier>
```

## Parameters
* **componentHash**: Weapon component name hash.

## Return value
Returns the weapon component accuracy modifier.
