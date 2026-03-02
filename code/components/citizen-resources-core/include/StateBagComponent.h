#pragma once

#include <ComponentHolder.h>

#include <optional>
#include <string_view>
#include <map>
#include <chrono>

#include <msgpack.hpp>

#include <SerializableComponent.h>

#include <SerializableProperty.h>
#include <SerializableStorageType.h>

#include "StateBag.h"

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define CRC_EXPORT DLL_EXPORT
#else
#define CRC_EXPORT DLL_IMPORT
#endif

namespace fx
{
class ResourceManager;

class CRC_EXPORT StateBagGameInterface
{
public:
	//
	// SendPacket should submit the passed data to the specified peer.
	//
	virtual void SendPacket(int peer, net::packet::StateBagPacket& packet) = 0;

	//
	// SendPacket should submit the passed data to the specified peer.
	//
	virtual void SendPacket(int peer, net::packet::StateBagV2Packet& packet) = 0;

	//
	// IsAsynchronous returns whether or not this game interface requires thread marshaling
	// before executing user code.
	//
	virtual bool IsAsynchronous()
	{
		return false;
	}

	//
	// QueueTask enqueues a task on the game interface's user code thread.
	//
	virtual void QueueTask(std::function<void()>&& task)
	{
		task();
	}
};

enum class StateBagRole
{
	Client,
	ClientV2,
	Server
};

class CRC_EXPORT StateBag : public std::enable_shared_from_this<StateBag>
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
	virtual void SetKey(int source, std::string_view key, std::string_view data, bool replicated = true) = 0;

	//
	// Sets the owning peer ID.
	//
	virtual void SetOwningPeer(std::optional<int> peer) = 0;
	
	//
	// Enable or disable immediate replication of this state bag's changes.
	// If enabled it'll send the state bag update immediatly (old behavior).
	// If disabled then new replicating state bag changes are queued, use SendQueuedUpdates() to send them.
	// #TODO: potentially remove this once the throttling system is in place
	//
	virtual void EnableImmediateReplication(bool enabled) = 0;

	//
	// Will send all queued replicating state bag updates to all targets, see EnableImmediateReplication(bool).
	//
	virtual void SendQueuedUpdates() = 0;

	//
	// Gets data for a key.
	//
	virtual std::optional<std::string> GetKey(std::string_view key) = 0;

	//
	// Returns whether the state bag has data for this key
	//
	virtual bool HasKey(std::string_view key) = 0;

	//
	// Returns all the keys that the state bag has data for
	//
	virtual std::vector<std::string> GetKeys() = 0;
};

class CRC_EXPORT StateBagComponent : public fwRefCountable, public IAttached<ResourceManager>
{
public:
	virtual ~StateBagComponent() override = default;

	//
	// Resets the state stored in the state bag component.
	//
	virtual void Reset() = 0;

	//
	// Old state bag control packet handler.
	// arg: outBagNameName; if given (!= nullptr) and if the state bag wasn't found then this string will contain the bag name, otherwise outBagNameName is unchanged.
	//
	virtual void HandlePacket(int source, std::string_view data, std::string* outBagNameName = nullptr) = 0;

	//
	// Should be called when receiving a state bag control packet.
	// arg: outBagNameName; if given (!= nullptr) and if the state bag wasn't found then this string will contain the bag name, otherwise outBagNameName is unchanged.
	//
	virtual void HandlePacketV2(int source, net::packet::StateBagV2& message, std::string_view* outBagNameName = nullptr) = 0;

	//
	// Gets a state bag by an identifier. Returns an empty shared_ptr if not found.
	//
	virtual std::shared_ptr<StateBag> GetStateBag(std::string_view id) = 0;

	//
	// Registers a state bag for the specified identifier. The pointer returned should be
	// the *only* reference, every reference kept internally should be weak.
	//
	virtual std::shared_ptr<StateBag> RegisterStateBag(std::string_view id, bool useParentTargets = false) = 0;

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

	//
	// Marks a given prefix as 'safe to pre-create'.
	//
	virtual void AddSafePreCreatePrefix(std::string_view idPrefix, bool useParentTargets) = 0;

	//
	// Returns the StateBagRole used for creation of the component
	//
	virtual StateBagRole GetRole() const = 0;
	
	//
	// Set the StateBagRole, used when the StateBagRole dynamically changes depending on protocol version between the server and client
	//
	virtual void SetRole(StateBagRole role) = 0;

	//
	// Methods for the debug UI
	//
	virtual void ClearOrphanedStateBags() = 0;
	virtual void ForceStateBagUpdates() = 0;
	virtual size_t GetStateBagCount() = 0;
	virtual size_t GetActiveStateBagCount() = 0;
	virtual size_t GetExpiredStateBagCount() = 0;
	virtual std::vector<std::string> GetStateBagIds() = 0;
	virtual size_t GetTargetCount() = 0;
	virtual size_t GetPreCreatedStateBagCount() = 0;
	virtual size_t GetErasureListCount() = 0;
	virtual size_t GetPreCreatePrefixCount() = 0;
	virtual bool IsGameInterfaceConnected() = 0;
	virtual const char* GetRoleName() = 0;
	virtual std::vector<int> GetRegisteredTargets() = 0;
	virtual size_t GetStateBagCountForTarget(int target) = 0;
	virtual std::vector<std::pair<std::string, bool>> GetPreCreatePrefixes() = 0;
	
	struct StateBagInfo {
		std::string id;
		bool isExpired;
		bool isActive;
		size_t keyCount;
	};
	
	struct StateBagDetails {
		std::string id;
		bool useParentTargets;
		bool replicationEnabled;
		std::optional<int> owningPeer;
		std::vector<int> routingTargets;
		std::map<std::string, std::string> storedData;
		std::map<std::string, size_t> queuedData; // key -> size
		bool isExpired;
	};
	
	virtual std::vector<StateBagInfo> GetStateBagInfoList() = 0;
	virtual bool IsStateBagExpired(const std::string& id) = 0;
	virtual std::optional<StateBagDetails> GetStateBagDetails(const std::string& id) = 0;
	virtual void SendStateBagQueuedUpdates(const std::string& id) = 0;

	struct RateLimitInfo {
		int clientId;
		uint32_t currentRate;      // Updates per second
		uint32_t burstCount;       // Current burst count
		uint32_t droppedUpdates;   // Total dropped updates
		std::string lastDroppedStateBag; // Last StateBag that caused a drop
		std::chrono::steady_clock::time_point lastDropTime;
		std::vector<std::string> recentStateBags; // Recent StateBags from this client
	};
	
	virtual std::vector<RateLimitInfo> GetRateLimitInfo() = 0;
	virtual uint32_t GetRateLimitRate() = 0;      // Current rate limit setting
	virtual uint32_t GetRateLimitBurst() = 0;     // Current burst limit setting
	virtual void ResetClientRateLimit(int clientId) = 0;
	virtual void TrackStateBagUpdate(int clientId, const std::string& stateBagId, bool wasDropped = false) = 0;
	
	// Method to track dropped updates from packet handlers
	virtual void TrackDroppedStateBagUpdate(int clientId, const std::string& stateBagId) = 0;

	//
	// An event handling a state bag value change.
	//
	fwEvent<int, std::string_view, std::string_view, const msgpack::object&, bool> OnStateBagChange;

public:
	static fwRefContainer<StateBagComponent> Create(StateBagRole role);
};
}

DECLARE_INSTANCE_TYPE(fx::StateBagComponent);
