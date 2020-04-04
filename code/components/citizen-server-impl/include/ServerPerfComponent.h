#pragma once

#include <prometheus/registry.h>

namespace fx
{
class ServerPerfComponent : public fwRefCountable
{
public:
	inline ServerPerfComponent()
	{
		m_registry = std::make_shared<prometheus::Registry>();
	}

	inline const std::shared_ptr<prometheus::Registry>& GetRegistry() const
	{
		return m_registry;
	}

private:
	std::shared_ptr<prometheus::Registry> m_registry;
};
}

DECLARE_INSTANCE_TYPE(fx::ServerPerfComponent);
