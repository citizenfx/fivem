---
ns: CFX
apiset: client
---
## SET_AUDIO_SUBMIX_OUTPUT_VOLUMES

```c
void SET_AUDIO_SUBMIX_OUTPUT_VOLUMES(int submixId, int outputSlot, float frontLeftVolume, float frontRightVolume, float rearLeftVolume, float rearRightVolume, float channel5Volume, float channel6Volume);
```

Sets the volumes for the sound channels in a submix effect.
Values can be between 0.0 and 1.0.
Channel 5 and channel 6 are not used in voice chat but are believed to be center and LFE channels.
Output slot starts at 0 for the first ADD\_AUDIO\_SUBMIX\_OUTPUT call then incremented by 1 on each subsequent call.

## Parameters
* **submixId**: The submix.
* **outputSlot**: The output slot index.
* **frontLeftVolume**: The volume for the front left channel.
* **frontRightVolume**: The volume for the front right channel.
* **rearLeftVolume**: The volume for the rear left channel.
* **rearRightVolume**: The volume for the rear right channel.
* **channel5Volume**: The volume for channel 5.
* **channel6Volume**: The volume for channel 6.
