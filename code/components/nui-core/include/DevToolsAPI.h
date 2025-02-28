#pragma once

#ifdef COMPILING_NUI_CORE
#define NUI_EXPORT __declspec(dllexport)
#else
#define NUI_EXPORT __declspec(dllimport)
#endif

extern "C" NUI_EXPORT void OpenNuiDevTools();
