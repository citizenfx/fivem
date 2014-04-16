#include "StdInc.h"
#include "Hooking.h"

namespace hook
{
	void inject_hook::inject()
	{
		inject_hook_frontend fe(this);
		m_assembly = std::make_shared<FunctionAssembly>(fe);

		put<uint8_t>(m_address, 0xE9);
		put<int>(m_address + 1, (uintptr_t)m_assembly->GetCode() - (uintptr_t)get_adjusted(m_address) - 5);
	}
}