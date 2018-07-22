#pragma once

namespace fx
{
	class ResourceGameLifetimeEvents : public fwRefCountable
	{
	public:
		fwEvent<> OnBeforeGameShutdown;

		fwEvent<> OnGameDisconnect;
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceGameLifetimeEvents);
