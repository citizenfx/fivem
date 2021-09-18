#pragma once

namespace ros
{
	std::string
#if defined(COMPILING_ROS_PATCHES_FIVE) || defined(COMPILING_ROS_PATCHES_RDR3)
		DLL_EXPORT
#endif
		GetEntitlementSource();
}
