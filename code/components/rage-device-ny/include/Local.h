#pragma once

#ifdef COMPILING_RAGE_DEVICE_NY
#undef COMPONENT_EXPORT
#define COMPONENT_EXPORT __declspec(dllexport)
#endif