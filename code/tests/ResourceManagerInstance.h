#pragma once

namespace fx
{
	class ResourceManager;

	class ResourceManagerInstance
	{
	public:
		static fx::ResourceManager* Create();
	};
}
