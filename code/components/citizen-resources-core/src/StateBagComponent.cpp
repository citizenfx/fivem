#include <StdInc.h>
#include <StateBagComponent.h>

#include <ResourceManager.h>

#include <shared_mutex>
#include <unordered_set>

//#include <EASTL/fixed_set.h>
#include <EASTL/fixed_vector.h>

#include <SharedFunction.h>
#include <state/RlMessageBuffer.h>

#include "ByteWriter.h"

namespace fx
{
class StateBagImpl;

class StateBagComponentImpl : public StateBagComponent
{
public:
	StateBagComponentImpl(StateBagRole role);

	virtual ~StateBagComponentImpl() override = default;

	virtual void AttachToObject(fx::ResourceManager* object) override;

	virtual void Reset() override;

	virtual void HandlePacket(int source, std::string_view data, std::string* outBagNameName = nullptr) override;

	virtual void HandlePacketV2(int source, net::packet::StateBagV2& message, std::string_view* outBagNameName = nullptr) override;

	virtual std::shared_ptr<StateBag> GetStateBag(std::string_view id) override;

	virtual std::shared_ptr<StateBag> RegisterStateBag(std::string_view id, bool useParentTargets = false) override;

	virtual void SetGameInterface(StateBagGameInterface* gi) override;

	virtual void RegisterTarget(int id) override;

	virtual void UnregisterTarget(int id) override;

	virtual void AddSafePreCreatePrefix(std::string_view idPrefix, bool useParentTargets) override;

	virtual StateBagRole GetRole() const override
	{
		return m_role;
	}

	virtual void SetRole(StateBagRole role) override
	{
		m_role = role;
	}

	void UnregisterStateBag(std::string_view id);

	// #TODO: this should eventually actually queue a send to allow for throttling
	void QueueSend(int target, net::packet::StateBagPacket& packet);

	void QueueSend(int target, net::packet::StateBagV2Packet& packet);

	inline std::tuple<std::shared_lock<std::shared_mutex>, std::reference_wrapper<const std::unordered_set<int>>> GetTargets()
	{
		return {
			std::shared_lock{ m_mapMutex },
			std::reference_wrapper{ m_targets }
		};
	}

	/// <summary>
	/// Checks if bag name is allowed to be automatically created
	/// </summary>
	/// <param name="id">full name of the state bag</param>
	/// <returns>first is if it exists, second determines if the state bag needs to use parent's targets</returns>
	std::pair<bool, bool> IsSafePreCreateName(std::string_view id);

	std::shared_ptr<StateBag> PreCreateStateBag(std::string_view id, bool useParentTargets);

	inline StateBagGameInterface* GetGameInterface() const
	{
		return m_gameInterface;
	}

private:
	StateBagGameInterface* m_gameInterface;

	StateBagRole m_role;

	std::unordered_set<int> m_targets;

	std::unordered_set<std::string> m_erasureList;
	std::shared_mutex m_erasureMutex;

	// TODO: evaluate transparent usage when we switch to C++20 compiler modes
	std::unordered_map<std::string, std::weak_ptr<StateBagImpl>> m_stateBags;
	std::shared_mutex m_mapMutex;

	// pre-created state bag stuff

	// list of state bag prefixes
	std::vector<std::pair<std::string, bool>> m_preCreatePrefixes;
	std::shared_mutex m_preCreatePrefixMutex;

	// *owning* pointers for pre-created bags
	std::set<std::shared_ptr<StateBagImpl>> m_preCreatedStateBags;
	std::shared_mutex m_preCreatedStateBagsMutex;
};

class StateBagImpl : public StateBag
{
public:
	StateBagImpl(StateBagComponentImpl* parent, std::string_view id, bool useParentTargets = false);

	virtual ~StateBagImpl() override;

	virtual std::optional<std::string> GetKey(std::string_view key) override;
	virtual bool HasKey(std::string_view key) override;
	virtual std::vector<std::string> GetKeys() override;
	virtual void SetKey(int source, std::string_view key, std::string_view data, bool replicated = true) override;
	virtual void SetRoutingTargets(const std::set<int>& peers) override;

	virtual void AddRoutingTarget(int peer) override;
	virtual void RemoveRoutingTarget(int peer) override;

	std::optional<int> GetOwningPeer();
	virtual void SetOwningPeer(std::optional<int> peer) override;

	// #TODO: potentially remove this once the throttling system is in place
	virtual void EnableImmediateReplication(bool enabled) override;
	virtual void SendQueuedUpdates() override;

	void SendKeyValue(int target, std::string_view key, std::string_view value);

	void SendKeyValueToAllTargets(std::string_view key, std::string_view value);

	void SendAll(int target);

	void SendAllInitial(int target);

private:
	void SetKeyInternal(int source, std::string_view key, std::string_view data, bool replicated);

private:
	StateBagComponentImpl* m_parent;
	bool m_useParentTargets;

	std::string m_id;

	std::shared_mutex m_routingTargetsMutex;
	std::set<int> m_routingTargets;

	std::optional<int> m_owningPeer;

	std::shared_mutex m_dataMutex;
	std::map<std::string, std::string, std::less<>> m_data;

	std::atomic<bool> m_replicationEnabled; //< #TODO: potentially remove this once the throttling system is in place
	std::mutex m_replicateDataMutex;
	std::unordered_map<std::string, std::string> m_replicateData;
};

StateBagImpl::StateBagImpl(StateBagComponentImpl* parent, std::string_view id, bool useParentTargets)
	: m_parent(parent), m_id(id), m_replicationEnabled(true), m_useParentTargets(useParentTargets)
{
	
}

StateBagImpl::~StateBagImpl()
{
	m_parent->UnregisterStateBag(m_id);
}

std::optional<std::string> StateBagImpl::GetKey(std::string_view key)
{
	std::shared_lock _(m_dataMutex);
	
	if (auto it = m_data.find(key); it != m_data.end())
	{
		return it->second;
	}

	return {};
}

std::vector<std::string> StateBagImpl::GetKeys()
{
	std::vector<std::string> keys;
	std::shared_lock _(m_dataMutex);

	for (auto data : m_data)
	{
		keys.push_back(data.first);
	}
	
	return keys;
}


bool StateBagImpl::HasKey(std::string_view key)
{
	std::shared_lock _(m_dataMutex);
	return m_data.count(key) != 0;
}

void StateBagImpl::SetKey(int source, std::string_view key, std::string_view data, bool replicated /* = true */)
{
	// prepare a potentially async continuation
	auto thisRef = shared_from_this();

	auto continuation = [thisRef, source, replicated](std::string_view key, std::string_view data)
	{
		static_cast<StateBagImpl*>(thisRef.get())->SetKeyInternal(source, key, data, replicated);
	};

	const auto& sbce = m_parent->OnStateBagChange;

	if (sbce)
	{
		// #TODOPERF: this will unconditionally unpack and call into svMain if *any* change handler is reg'd
		// -> ideally we'd have a separate event so we can poll if there's a match for script filters before doing this
		msgpack::unpacked up;

		try
		{
			up = msgpack::unpack(data.data(), data.size());
		}
		catch (...)
		{
			return;
		}

		auto gameInterface = m_parent->GetGameInterface();

		if (gameInterface->IsAsynchronous())
		{
			const auto& id = m_id;
			auto parent = m_parent;
			std::string keyStr{ key };
			std::string dataStr{ data };

			gameInterface->QueueTask(make_shared_function([parent,
				source,
				replicated,
				id,
				continuation = std::move(continuation),
				key = std::move(keyStr),
				data = std::move(dataStr),
				up = std::move(up)
			]()
			{
				if (parent->OnStateBagChange(source, id, key, up.get(), replicated))
				{
					continuation(key, data);
				}
			}));

			return;
		}

		if (!sbce(source, m_id, key, up.get(), replicated))
		{
			return;
		}
	}

	continuation(key, data);
}

// https://github.com/msgpack/msgpack/blob/master/spec.md#formats
constexpr char MsgPackNil = static_cast<char>(0xc0);
void StateBagImpl::SetKeyInternal(int source, std::string_view key, std::string_view data, bool replicated)
{
	{
		std::unique_lock _(m_dataMutex);
		if (data[0] == MsgPackNil)
		{
			m_data.erase(std::string { key });
		}
		else if (auto it = m_data.find(key); it != m_data.end())
		{
			if (data != it->second)
			{
				it->second = data;
			}
			else
			{
				replicated = false; // nothing changed
			}
		}
		else
		{
			m_data.emplace(key, data);
		}
	}

	if (replicated)
	{
		if (m_replicationEnabled)
		{
			SendKeyValueToAllTargets(key, data);
		}
		else
		{
			// store it for now
			std::unique_lock lk(m_replicateDataMutex);
			m_replicateData.emplace(key, data);
		}
	}
}

std::optional<int> StateBagImpl::GetOwningPeer()
{
	return m_owningPeer;
}

void StateBagImpl::SetOwningPeer(std::optional<int> peer)
{
	m_owningPeer = peer;
}

void StateBagImpl::EnableImmediateReplication(bool enabled)
{
	m_replicationEnabled = enabled;
}

void StateBagImpl::SendQueuedUpdates()
{
	decltype(m_replicateData) replicationData;

	{
		std::unique_lock lk(m_replicateDataMutex);
		m_replicateData.swap(replicationData);
	}

	for (auto& [key, value] : replicationData)
	{
		SendKeyValueToAllTargets(key, value);
	}
}

void StateBagImpl::SendAll(int target)
{
	// string_view's target memory can get removed in the mean time, so let's keep locking it instead
	//eastl::fixed_set<std::string_view, 50> keys;

	{
		std::shared_lock _(m_dataMutex);

		for (auto& [ key, value ] : m_data)
		{
			SendKeyValue(target, key, value);
		}
	}
}

void StateBagImpl::SendAllInitial(int target)
{
	{
		std::shared_lock _(m_routingTargetsMutex);
		if (!m_routingTargets.empty() && m_routingTargets.find(target) == m_routingTargets.end())
		{
			return;
		}
	}

	SendAll(target);
}

void StateBagImpl::SendKeyValueToAllTargets(std::string_view key, std::string_view value)
{
	if (m_useParentTargets)
	{
		auto [lock, refTargets] = m_parent->GetTargets();
		auto targets = refTargets.get();

		for (int target : targets)
		{
			SendKeyValue(target, key, value);
		}
	}
	else
	{
		std::shared_lock _(m_routingTargetsMutex);

		for (int target : m_routingTargets)
		{
			SendKeyValue(target, key, value);
		}
	}
}

void StateBagImpl::SendKeyValue(int target, std::string_view key, std::string_view value)
{
	// new server will accept this message
	if (m_parent->GetRole() == StateBagRole::ClientV2)
	{
		if (!key.empty() && !value.empty())
		{
			net::packet::StateBagV2Packet stateBagPacket;
			stateBagPacket.data.stateBagName = m_id;
			stateBagPacket.data.key = key;
			stateBagPacket.data.data = value;
			m_parent->QueueSend(target, stateBagPacket);
		}
	}
	else
	{
		// client connecting to a older server need to send this one
		// server need to send this one, because he has no way to know version of the client
		static thread_local rl::MessageBuffer dataBuffer(131072);

		if (!key.empty() && !value.empty())
		{
			dataBuffer.SetCurrentBit(0);

			auto writeStr = [](const auto& str)
			{
				dataBuffer.Write<uint16_t>(16, str.size() + 1);
				dataBuffer.WriteBits(str.data(), str.size() * 8);
				dataBuffer.Write<uint8_t>(8, 0);
			};

			{
				writeStr(m_id);
				writeStr(key);
				dataBuffer.WriteBits(value.data(), value.size() * 8);
			}

			net::packet::StateBagPacket stateBagPacket;
			stateBagPacket.data.data = std::string_view{ reinterpret_cast<const char*>(dataBuffer.GetBuffer().data()), dataBuffer.GetCurrentBit() / 8 };
			m_parent->QueueSend(target, stateBagPacket);
		}
	}
}

void StateBagImpl::SetRoutingTargets(const std::set<int>& peers)
{
	std::unique_lock _(m_routingTargetsMutex);
	auto oldSet = std::move(m_routingTargets);
	m_routingTargets = peers;

	eastl::fixed_vector<int, 256> newTargets, goneTargets;
	std::set_difference(m_routingTargets.begin(), m_routingTargets.end(), oldSet.begin(), oldSet.end(), std::back_inserter(newTargets));
	std::set_difference(oldSet.begin(), oldSet.end(), m_routingTargets.begin(), m_routingTargets.end(), std::back_inserter(goneTargets));

	for (int target : goneTargets)
	{
		// send some sort of deletion if needed
	}

	for (int target : newTargets)
	{
		SendAll(target);
	}
}

void StateBagImpl::AddRoutingTarget(int peer)
{
	std::unique_lock _(m_routingTargetsMutex);
	auto [it, succ] = m_routingTargets.insert(peer);

	if (succ)
	{
		SendAll(peer);
	}
}

void StateBagImpl::RemoveRoutingTarget(int peer)
{
	std::unique_lock _(m_routingTargetsMutex);
	m_routingTargets.erase(peer);
}

StateBagComponentImpl::StateBagComponentImpl(StateBagRole role)
	: m_gameInterface(nullptr), m_role(role)
{

}

void StateBagComponentImpl::SetGameInterface(StateBagGameInterface* gi)
{
	m_gameInterface = gi;
}

std::shared_ptr<StateBag> StateBagComponentImpl::RegisterStateBag(std::string_view id, bool useParentTargets)
{
	std::shared_ptr<StateBagImpl> bag;
	std::string strId{ id };

	{
		std::unique_lock lock(m_mapMutex);

		if (auto exIt = m_stateBags.find(strId); exIt != m_stateBags.end())
		{
			auto bagRef = exIt->second.lock();

			if (bagRef)
			{
				lock.unlock();

				// disown pre-created state bag reference, if this one came from there
				{
					std::unique_lock preLock(m_preCreatedStateBagsMutex);
					m_preCreatedStateBags.erase(bagRef);
				}

				return bagRef;
			}
		}

		// *only* start making a new one here as otherwise we'll delete the existing bag
		bag = std::make_shared<StateBagImpl>(this, id, useParentTargets);
		m_stateBags[strId] = bag;
	}

	{
		std::unique_lock _(m_erasureMutex);
		m_erasureList.erase(strId);
	}

	return bag;
}

void StateBagComponentImpl::UnregisterStateBag(std::string_view id)
{
	std::unique_lock _(m_erasureMutex);
	m_erasureList.emplace(id);
}

std::shared_ptr<StateBag> StateBagComponentImpl::GetStateBag(std::string_view id)
{
	std::shared_lock lock(m_mapMutex);
	// unfortunately this string allocation is required, because we are not using cpp20 yet
	// see: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r2.html
	// TODO: remove std::string allocation when cpp20 is used
	auto bag = m_stateBags.find(std::string{ id });

	return (bag != m_stateBags.end()) ? bag->second.lock() : nullptr;
}

void StateBagComponentImpl::RegisterTarget(int id)
{
	bool isNew = false;

	{
		std::unique_lock lock(m_mapMutex);

		if (m_targets.find(id) == m_targets.end())
		{
			isNew = true;
		}

		m_targets.insert(id);
	}

	if (isNew)
	{
		std::shared_lock lock(m_mapMutex);

		for (auto& data : m_stateBags)
		{
			auto entry = data.second.lock();

			if (entry)
			{
				entry->SendAllInitial(id);
			}
		}
	}
}

void StateBagComponentImpl::UnregisterTarget(int id)
{
	std::unique_lock lock(m_mapMutex);
	m_targets.erase(id);

	for (auto& bag : m_stateBags)
	{
		auto b = bag.second.lock();

		if (b)
		{
			b->RemoveRoutingTarget(id);
		}
	}
}

void StateBagComponentImpl::AddSafePreCreatePrefix(std::string_view idPrefix, bool useParentTargets)
{
	std::unique_lock lock(m_preCreatePrefixMutex);
	m_preCreatePrefixes.emplace_back(idPrefix, useParentTargets);
}

std::pair<bool, bool> StateBagComponentImpl::IsSafePreCreateName(std::string_view id)
{
	std::shared_lock lock(m_preCreatePrefixMutex);

	for (const auto& prefix : m_preCreatePrefixes)
	{
		if (id.rfind(prefix.first, 0) == 0) // equivalent of a starts with
		{
			return { true, prefix.second };
		}
	}

	return { false, false };
}

std::shared_ptr<StateBag> StateBagComponentImpl::PreCreateStateBag(std::string_view id, bool useParentTargets)
{
	auto bag = RegisterStateBag(id, useParentTargets);

	{
		std::unique_lock lock(m_preCreatedStateBagsMutex);
		m_preCreatedStateBags.insert(std::static_pointer_cast<StateBagImpl>(bag));
	}

	return bag;
}

void StateBagComponentImpl::AttachToObject(fx::ResourceManager* object)
{
	object->OnTick.Connect([this]()
	{
		bool isEmpty = true;

		{
			std::shared_lock _(m_erasureMutex);
			isEmpty = m_erasureList.empty();
		}

		if (isEmpty)
		{
			return;
		}

		decltype(m_erasureList) erasureList;

		{
			std::unique_lock _(m_erasureMutex);
			erasureList = std::move(m_erasureList);
		}

		if (!erasureList.empty())
		{
			std::unique_lock lock(m_mapMutex);

			for (const auto& id : erasureList)
			{
				m_stateBags.erase(id);
			}
		}
	},
	INT32_MIN);
}

void StateBagComponentImpl::HandlePacket(int source, std::string_view dataRaw, std::string* outBagNameName)
{
	// read state
	rl::MessageBuffer buffer{ reinterpret_cast<const uint8_t*>(dataRaw.data()), dataRaw.size() };

	auto readStr = [&buffer]()
	{
		uint16_t length = 0;

		buffer.Read<uint16_t>(16, &length);

		// validate input
		if (length <= 0 || length > std::numeric_limits<uint16_t>::max())
		{
			return std::vector<char>{};
		}

		std::vector<char> rbuffer(length - 1);
		buffer.ReadBits(rbuffer.data(), rbuffer.size() * 8);
		buffer.Read<uint8_t>(8);

		return std::move(rbuffer);
	};

	// read id
	auto idNameBuffer = readStr();

	if (idNameBuffer.empty())
	{
		return;
	}

	// read key
	auto keyBuffer = readStr();

	if (keyBuffer.empty())
	{
		return;
	}

	// if m_curBit is greater then m_maxBit we will overflow the dataLength, which would lead to an allocation of an
	// extremely large buffer, which would fail and crash the server.
	if (buffer.IsAtEnd())
	{
		return;
	}

	// read data
	size_t dataLength = (buffer.GetLength() * 8) - buffer.GetCurrentBit();

	if (dataLength == 0)
	{
		return;
	}

	std::vector<uint8_t> data((dataLength + 7) / 8);
	buffer.ReadBits(data.data(), dataLength);

	// handle data
	auto bagName = std::string_view{
		idNameBuffer.data(), idNameBuffer.size()
	};

	auto bag = GetStateBag(bagName);

	if (!bag)
	{
		auto safeToCreate = IsSafePreCreateName(bagName);
		if (safeToCreate.first)
		{
			bag = PreCreateStateBag(bagName, safeToCreate.second);
		}
	}

	if (bag)
	{
		auto bagRef = std::static_pointer_cast<StateBagImpl>(bag);

		// TODO: rate checks, policy checks
		auto peer = bagRef->GetOwningPeer();
		if (!peer.has_value() || source == *peer)
		{
			bagRef->SetKey(
				source,
				std::string_view{ keyBuffer.data(), keyBuffer.size() },
				std::string_view{ reinterpret_cast<char*>(data.data()), data.size() },
				m_role == StateBagRole::Server);
		}		
	}
	else if(outBagNameName != nullptr)
	{
		*outBagNameName = bagName;
	}
}

void StateBagComponentImpl::HandlePacketV2(int source, net::packet::StateBagV2& message, std::string_view* outBagNameName)
{
	if (message.stateBagName.GetValue().empty() || message.key.GetValue().empty() || message.data.GetValue().empty())
	{
		return;
	}

	auto bag = GetStateBag(message.stateBagName);

	if (!bag)
	{
		if (const auto safeToCreate = IsSafePreCreateName(message.stateBagName); safeToCreate.first)
		{
			bag = PreCreateStateBag(message.stateBagName, safeToCreate.second);
		}
	}

	if (bag)
	{
		const auto bagRef = std::static_pointer_cast<StateBagImpl>(bag);

		// TODO: rate checks, policy checks
		const auto peer = bagRef->GetOwningPeer();
		if (!peer.has_value() || source == *peer)
		{
			bagRef->SetKey(
				source,
				message.key,
				message.data,
				m_role == StateBagRole::Server);
		}		
	}
	else if (outBagNameName != nullptr)
	{
		*outBagNameName = message.stateBagName;
	}
}

void StateBagComponentImpl::QueueSend(int target, net::packet::StateBagPacket& packet)
{
	m_gameInterface->SendPacket(target, packet);
}

void StateBagComponentImpl::QueueSend(int target, net::packet::StateBagV2Packet& packet)
{
	m_gameInterface->SendPacket(target, packet);
}

void StateBagComponentImpl::Reset()
{
	{
		std::unique_lock _(m_mapMutex);
		m_stateBags.clear();
	}

	{
		std::unique_lock _(m_preCreatedStateBagsMutex);
		m_preCreatedStateBags.clear();
	}
}

fwRefContainer<StateBagComponent> StateBagComponent::Create(StateBagRole role)
{
	return new StateBagComponentImpl(role);
}
}
