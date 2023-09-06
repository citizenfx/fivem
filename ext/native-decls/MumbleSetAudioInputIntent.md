---
ns: CFX
apiset: client
---
## MUMBLE_SET_AUDIO_INPUT_INTENT

```c
void MUMBLE_SET_AUDIO_INPUT_INTENT(Hash intentHash);
```
Use this native to disable noise suppression and high pass filters.

The possible intents for this are as follows (backticks are used to represent hashes):

| Index | Description |
|-|-|
| \`speech\` | Default intent |
| \`music\` | Disable noise suppression and high pass filter |

## Parameters
* **intentHash**: The intent hash.

## Examples
```lua
-- disable noise suppression and high pass filter
MumbleSetAudioInputIntent(`music`)

-- set the default intent (enable noise suppression and high pass filter)
MumbleSetAudioInputIntent(`speech`)
```