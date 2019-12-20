#pragma once

extern
#ifdef COMPILING_GTA_CORE_RDR3
DLL_EXPORT
#else
DLL_IMPORT
#endif
	fwEvent<> OnMsgConfirm;
