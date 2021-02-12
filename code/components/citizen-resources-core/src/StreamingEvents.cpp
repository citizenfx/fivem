#include <StdInc.h>
#include <StreamingEvents.h>

namespace fx
{
	DLL_EXPORT fwEvent<const std::string&, size_t, size_t> OnCacheDownloadStatus;
	DLL_EXPORT fwEvent<const std::string&, size_t, size_t> OnCacheVerifyStatus;
}
