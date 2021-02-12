#pragma once

namespace fx
{
	extern
#ifdef COMPILING_CITIZEN_RESOURCES_CORE
		DLL_EXPORT
#else
		DLL_IMPORT
#endif
		fwEvent<const std::string&, size_t, size_t> OnCacheDownloadStatus;

	extern
#ifdef COMPILING_CITIZEN_RESOURCES_CORE
		DLL_EXPORT
#else
		DLL_IMPORT
#endif
		fwEvent<const std::string&, size_t, size_t> OnCacheVerifyStatus;
}
