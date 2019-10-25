#pragma once

#include <ComponentHolder.h>

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define CRC_EXPORT DLL_EXPORT
#else
#define CRC_EXPORT DLL_IMPORT
#endif

namespace fx
{
class Resource;
class ResourceManager;

class EventReassemblySink
{
public:
	virtual void SendPacket(int target, std::string_view data) = 0;
};

class CRC_EXPORT EventReassemblyComponent : public fwRefCountable, public IAttached<ResourceManager>
{
public:
	virtual ~EventReassemblyComponent() override = default;

	virtual void SetSink(EventReassemblySink* sink) = 0;

	virtual void RegisterTarget(int id) = 0;

	virtual void UnregisterTarget(int id) = 0;

	virtual void HandlePacket(int source, std::string_view data) = 0;

	virtual void TriggerEvent(int target, std::string_view eventName, std::string_view eventPayload, int bytesPerSecond = 50000) = 0;

	virtual void NetworkTick() = 0;

public:
	static fwRefContainer<EventReassemblyComponent> Create();
};
}

DECLARE_INSTANCE_TYPE(fx::EventReassemblyComponent);
