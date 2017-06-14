#pragma once

namespace ros
{
	std::string
#ifdef COMPILING_ROS_PATCHES_FIVE
		DLL_EXPORT
#endif
		GetEntitlementSource();
}
