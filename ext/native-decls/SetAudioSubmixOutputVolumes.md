---
ns: CFX
apiset: client
game: gta5
---
## SET_AUDIO_SUBMIX_OUTPUT_VOLUMES

```c
void SET_AUDIO_SUBMIX_OUTPUT_VOLUMES(int submixId, int effectSlot, float leftVolume, float rightVolume, float rearLeftVolume, float rearRightVolume, float centerVolume, float lfeVolume);
```

Sets the volumes for the sound channels in a submix effect.
Values can be between 0.0 and 1.0.

## Parameters
* **submixId**: The submix.
* **effectSlot**: The effect slot for the submix. It is expected that the effect is set in this slot beforehand.
* **leftVolume**: The volume for the left channel.
* **rightVolume**: The volume for the right channel.
* **rearLeftVolume**: The volume for the left surround channel.
* **rearRightVolume**: The volume for the right surround channel.
* **centerVolume**: The volume for the center channel.
* **lfeVolume**: The volume for the low-frequency effects channel.
