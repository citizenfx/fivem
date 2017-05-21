#pragma once

#include <ServerInstanceBase.h>

namespace fx
{
	class ServerInstanceBaseRef : public fwRefCountable
	{
	public:
		inline ServerInstanceBaseRef(ServerInstanceBase* instance)
			: m_ref(instance)
		{

		}

		inline ServerInstanceBase* Get()
		{
			return m_ref;
		}

	private:
		ServerInstanceBase* m_ref;
	};
}

DECLARE_INSTANCE_TYPE(fx::ServerInstanceBaseRef);
