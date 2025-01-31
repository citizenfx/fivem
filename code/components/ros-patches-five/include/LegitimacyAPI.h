#pragma once

namespace ros
{
	std::string
// #TODO: when making COMPONENT_EXPORT macro, also define prefix variants
#if defined(COMPILING_ROS_PATCHES_FIVE) || defined(COMPILING_ROS_PATCHES_RDR3) || defined(COMPILING_ROS_PATCHES_NY)
		DLL_EXPORT
#endif
		GetEntitlementSource();
}
