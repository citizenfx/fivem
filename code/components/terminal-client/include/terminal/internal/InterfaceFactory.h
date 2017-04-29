/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <unordered_map>

namespace terminal
{
class InterfaceFactory
{
public:
	typedef fwRefContainer<fwRefCountable>(*FactoryFn)();

private:
	std::unordered_map<uint64_t, FactoryFn> m_typeFactories;

public:
	fwRefContainer<fwRefCountable> CreateInterface(uint64_t interfaceId);

	void RegisterInterface(uint64_t interfaceId, FactoryFn factory);
};

#define REGISTER_INTERFACE(x) \
	static InitFunction interfaceInit_##x([] () \
	{ \
		Instance<InterfaceFactory>::Get()->RegisterInterface(x::InterfaceID, [] () -> fwRefContainer<fwRefCountable> { return new x(); }); \
	});
}

DECLARE_INSTANCE_TYPE(terminal::InterfaceFactory);