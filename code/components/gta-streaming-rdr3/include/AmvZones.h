#pragma once

#include <string>

namespace streaming
{
	// Collects a file a resource streams as an ambient mask volume zone, if its name (without the extension) starts
	// with "amv_zone_".
	void AddAmvZoneFile(const std::string& baseName);
}
