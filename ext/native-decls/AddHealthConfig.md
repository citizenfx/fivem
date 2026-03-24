---
ns: CFX
apiset: client
game: gta5
---
## ADD_HEALTH_CONFIG

```c
void ADD_HEALTH_CONFIG(char* configName, float defaultHealth, float defaultArmor, float defaultEndurance, float fatiguedHealthThreshold, float injuredHealthThreshold, float dyingHealthThreshold, float hurtHealthThreshold, float dogTakedownThreshold, float writheFromBulletThreshold, BOOL meleeCardinalFatalAttack, BOOL invincible);
```

Adds new health config.

## Parameters
* **configName**: Name of health config. Cannot be default game health config name.
* **defaultHealth**: Default health value.
* **defaultArmor**: Default armor value.
* **fatiguedHealthThreshold**: Fatigued health threshold value.
* **injuredHealthThreshold**: Injured health threshold value.
* **dyingHealthThreshold**: Dying health threshold value.
* **fatiguedHealthThreshold**: Fatigued health threshold value.
* **hurtHealthThreshold**: Hurt health threshold value.
* **dogTakedownThreshold**: Dog takedown threshold value.
* **writheFromBulletThreshold**: Writhe from bulled threshold value.
* **meleeCardinalFatalAttack**: Melee cardinal fatal attack check value.
* **invincible**: Invincible value.

