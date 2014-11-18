#pragma once

#ifndef GTACORE_EXPORT
#ifdef COMPILING_GTA_CORE_NY
#define GTACORE_EXPORT __declspec(dllexport)
#else
#define GTACORE_EXPORT __declspec(dllimport)
#endif
#endif

extern GTACORE_EXPORT fwEvent<> OnMsgConfirm;

extern GTACORE_EXPORT fwEvent<> OnPostFrontendRender;