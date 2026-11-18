#pragma once

#ifdef COMPILING_RAGE_GRAPHICS_FIVE
#define SCENE_LUMINANCE_EXPORT __declspec(dllexport)
#else
#define SCENE_LUMINANCE_EXPORT __declspec(dllimport)
#endif

bool SCENE_LUMINANCE_EXPORT GetSceneLuminance(float left, float top, float right, float bottom, float& value);
