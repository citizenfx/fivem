#pragma once

#include <mono/metadata/appdomain.h>

namespace fx::mono
{
	class MonoDomainScope
	{
	private:
		MonoDomain* prevDomain;

	public:
		MonoDomainScope(MonoDomain* domain)
			: prevDomain(mono_domain_get())
		{
			mono_domain_set_internal(domain);
		}

		~MonoDomainScope()
		{
			mono_domain_set_internal(prevDomain);
		}
	};
}
