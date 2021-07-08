#pragma once

#ifdef COMPILING_GTA_STREAMING_FIVE
#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

STREAMING_EXPORT fwEvent<> OnRefreshArchetypesCollection;
STREAMING_EXPORT fwEvent<> OnRefreshArchetypesCollectionDone;
