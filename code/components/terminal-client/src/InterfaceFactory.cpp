/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/InterfaceFactory.h>

namespace terminal
{
fwRefContainer<fwRefCountable> InterfaceFactory::CreateInterface(uint64_t interfaceId)
{
	auto it = m_typeFactories.find(interfaceId);
	fwRefContainer<fwRefCountable> interfacePtr;

	if (it != m_typeFactories.end())
	{
		interfacePtr = it->second();
	}

	return interfacePtr;
}

void InterfaceFactory::RegisterInterface(uint64_t interfaceId, FactoryFn factory)
{
	m_typeFactories[interfaceId] = factory;
}

static InitFunction initFunction([] ()
{
	Instance<InterfaceFactory>::Set(new InterfaceFactory());
}, -10000);

fwRefContainer<fwRefCountable> CreateInterface(uint64_t interfaceID)
{
	return Instance<InterfaceFactory>::Get()->CreateInterface(interfaceID);
}
}