---
ns: CFX
apiset: client
---
## SET_AUDIO_SUBMIX_EFFECT_RADIO_FX

```c
void SET_AUDIO_SUBMIX_EFFECT_RADIO_FX(int submixId, int effectSlot);
```

Assigns a RadioFX effect to a submix effect slot.

The parameter values for this effect are as follows (backticks are used to represent hashes):

| Index | Type | Description |
|-|-|-|
| \`enabled\` | int | Enables or disables RadioFX on this DSP. |
| \`default\` | int | Sets default parameters for the RadioFX DSP and enables it. |
| \`freq_low\` | float |  |
| \`freq_hi\` | float |  |
| \`fudge\` | float |  |
| \`rm_mod_freq\` | float |  |
| \`rm_mix\` | float |  |
| \`o_freq_lo\` | float |  |
| \`o_freq_hi\` | float |  |

## Parameters
* **submixId**: The submix.
* **effectSlot**: The effect slot for the submix.

## Examples
```lua
-- we want to change the master output
local submix = 0

-- add a RadioFX effect to slot 0
SetAudioSubmixEffectRadioFx(submix, 0)

-- set the default values
SetAudioSubmixEffectParamInt(submix, 0, `default`, 1)
```