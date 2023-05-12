---
ns: CFX
apiset: client
---
## SET_AUDIO_SUBMIX_EFFECT_PARAM_FLOAT

```c
void SET_AUDIO_SUBMIX_EFFECT_PARAM_FLOAT(int submixId, int effectSlot, int paramIndex, float paramValue);
```

Sets a floating-point parameter for a submix effect.

## Parameters
* **submixId**: The submix.
* **effectSlot**: The effect slot for the submix. It is expected that the effect is set in this slot beforehand.
* **paramIndex**: The parameter index for the effect.
* **paramValue**: The parameter value to set.
