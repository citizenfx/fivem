---
ns: CFX
apiset: client
game: gta5
---
## GET_GAME_SETTINGS

```c
object GET_GAME_SETTINGS();
```

Returns the game's active graphics and video settings, as stored in `settings.xml`. This allows resources to build
their own settings menus.

Values are integers, floats or booleans depending on the setting. Available keys:

* Graphics: `Tessellation`, `LodScale`, `PedLodBias`, `VehicleLodBias`, `ShadowQuality`, `ReflectionQuality`,
  `ReflectionMSAA`, `SSAO`, `AnisotropicFiltering`, `MSAA`, `MSAAFragments`, `MSAAQuality`, `SamplingMode`,
  `TextureQuality`, `ParticleQuality`, `WaterQuality`, `GrassQuality`, `ShaderQuality`, `Shadow_SoftShadows`,
  `UltraShadows_Enabled`, `Shadow_ParticleShadows`, `Shadow_Distance`, `Shadow_LongShadows`, `Shadow_SplitZStart`,
  `Shadow_SplitZEnd`, `Shadow_aircraftExpWeight`, `Shadow_DisableScreenSizeCheck`, `Reflection_MipBlur`,
  `FXAA_Enabled`, `TXAA_Enabled`, `Lighting_FogVolumes`, `Shader_SSA`, `DX_Version`, `CityDensity`,
  `PedVarietyMultiplier`, `VehicleVarietyMultiplier`, `PostFX`, `DoF`, `HdStreamingInFlight`, `MaxLodScale`,
  `MotionBlurStrength`
* Video: `AdapterIndex`, `OutputIndex`, `ScreenWidth`, `ScreenHeight`, `RefreshRate`, `Windowed`, `VSync`, `Stereo`,
  `Convergence`, `Separation`, `PauseOnFocusLoss`, `AspectRatio`

## Return value
A table of setting names to their current values.

## Examples
```lua
local settings = GetGameSettings()
print(('%dx%d, texture quality %d'):format(settings.ScreenWidth, settings.ScreenHeight, settings.TextureQuality))
```
