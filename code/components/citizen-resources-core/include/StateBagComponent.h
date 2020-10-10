#pragma once

#include <ComponentHolder.h>

#include <optional>
#include <string_view>

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define CRC_EXPORT DLL_EXPORT
#else
#define CRC_EXPORT DLL_IMPORT
#endif

namespace fx
{
class ResourceManager;

class StateBagGameInterface
{
public:
	//
	// SendPacket should submit the passed data to the specified peer.
	//
	virtual void SendPacket(int peer, std::string_view data) = 0;
};

enum class StateBagRole
{
	Client,
	Server
};

class CRC_EXPORT StateBag
{
public:
	virtual ~StateBag() = default;

	//
	// Adds a routing peer.
	//
	virtual void AddRoutingTarget(int peer) = 0;

	//
	// Removes a routing peer.
	//
	virtual void RemoveRoutingTarget(int peer) = 0;

	//
	// Overrides the set of peers to route to.
	//
	virtual void SetRoutingTargets(const std::set<int>& peers) = 0;

	//
	// Sets data for a key.
	//
	virtual void SetKey(std::string_view key, std::string_view data, bool replicated = true) = 0;

	//
	// Sets the owning peer ID.
	//
	virtual void SetOwningPeer(std::optional<int> peer) = 0;

	//
	// Gets data for a key.
	//
	virtual std::optional<std::string> GetKey(std::string_view key) = 0;
};

class CRC_EXPORT StateBagComponent : public fwRefCountable, public IAttached<ResourceManager>
{
public:
	virtual ~StateBagComponent() override = default;

	//
	// Should be called when receiving a state bag control packet.
	//
	virtual void HandlePacket(int source, std::string_view data) = 0;

	//
	// Gets a state bag by an identifier. Returns an empty shared_ptr if not found.
	//
	virtual std::shared_ptr<StateBag> GetStateBag(std::string_view id) = 0;

	//
	// Registers a state bag for the specified identifier. The pointer returned should be
	// the *only* reference, every reference kept internally should be weak.
	//
	virtual std::shared_ptr<StateBag> RegisterStateBag(std::string_view id) = 0;

	//
	// Sets the game interface for sending network packets.
	//
	virtual void SetGameInterface(StateBagGameInterface* gi) = 0;

	//
	// Registers a broadcast target.
	//
	virtual void RegisterTarget(int id) = 0;

	//
	// Marks a broadcast target as removed.
	//
	virtual void UnregisterTarget(int id) = 0;

public:
	static fwRefContainer<StateBagComponent> Create(StateBagRole role);
};
}

DECLARE_INSTANCE_TYPE(fx::StateBagComponent);
