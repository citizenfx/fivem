#include "StdInc.h"
#include "BuildInfo.h"
#include <cfx_version.h>

int fx::BuildInfo::GetBuildNumber()
{
	const char* lastPeriod = strchr(GIT_TAG, '.');
	return lastPeriod == nullptr ? 0 : strtol(lastPeriod + 1, nullptr, 10);
}
