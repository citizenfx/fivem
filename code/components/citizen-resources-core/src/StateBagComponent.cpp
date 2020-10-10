#include <StdInc.h>
#include <StateBagComponent.h>

#include <shared_mutex>
#include <unordered_set>

#include <EASTL/fixed_set.h>
#include <EASTL/fixed_vector.h>

#include <state/RlMessageBuffer.h>

namespace fx
{
class StateBagImpl;

class StateBagComponentImpl : public StateBagComponent
{
public:
	StateBagComponentImpl(StateBagRole role);

	virtual ~StateBagComponentImpl() override = default;

	virtual void AttachToObject(fx::ResourceManager* object) override;

	virtual void HandlePacket(int source, std::string_view data) override;

	virtual std::shared_ptr<StateBag> GetStateBag(std::string_view id) override;

	virtual std::shared_ptr<StateBag> RegisterStateBag(std::string_view id) override;

	virtual void SetGameInterface(StateBagGameInterface* gi) override;

	virtual void RegisterTarget(int id) override;

	virtual void UnregisterTarget(int id) override;

	void UnregisterStateBag(std::string_view id);

	// #TODO: this should eventually actually queue a send to allow for throttling
	void QueueSend(int target, std::string_view packet);

	inline std::tuple<std::shared_lock<std::shared_mutex>, std::reference_wrapper<const std::unordered_set<int>>> GetTargets()
	{
		return {
			std::shared_lock{ m_mapMutex },
			std::reference_wrapper{ m_targets }
		};
	}

private:
	StateBagGameInterface* m_gameInterface;

	StateBagRole m_role;

	std::unordered_set<int> m_targets;

	// TODO: evaluate transparent usage when we switch to C++20 compiler modes
	std::unordered_map<std::string, std::weak_ptr<StateBagImpl>> m_stateBags;
	std::shared_mutex m_mapMutex;
};

class StateBagImpl : public StateBag
{
public:
	StateBagImpl(StateBagComponentImpl* parent, std::string_view id);

	virtual ~StateBagImpl() override;

	virtual std::optional<std::string> GetKey(std::string_view key) override;
	virtual void SetKey(std::string_view key, std::string_view data, bool replicated = true) override;
	virtual void SetRoutingTargets(const std::set<int>& peers) override;

	virtual void AddRoutingTarget(int peer) override;
	virtual void RemoveRoutingTarget(int peer) override;

	std::optional<int> GetOwningPeer();
	virtual void SetOwningPeer(std::optional<int> peer) override;

	void SendKey(int target, std::string_view key);

	void SendKeyAll(std::string_view key);

	void SendAll(int target);

private:
	StateBagComponentImpl* m_parent;

	std::string m_id;

	std::shared_mutex m_routingTargetsMutex;
	std::set<int> m_routingTargets;

	std::optional<int> m_owningPeer;

	std::shared_mutex m_dataMutex;
	std::map<std::string, std::string, std::less<>> m_data;
};

StateBagImpl::StateBagImpl(StateBagComponentImpl* parent, std::string_view id)
	: m_parent(parent), m_id(id)
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

void StateBagImpl::SetKey(std::string_view key, std::string_view data, bool replicated /* = true */)
{
	std::string lastValue;

	{
		std::unique_lock _(m_dataMutex);

		if (replicated)
		{
			if (auto it = m_data.find(key); it != m_data.end())
			{
				lastValue = std::move(it->second);
			}
		}

		m_data[std::string{ key }] = std::string{ data };
	}

	if (replicated)
	{
		if (data != lastValue)
		{
			SendKeyAll(key);
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

void StateBagImpl::SendAll(int target)
{
	eastl::fixed_set<std::string_view, 50> keys;

	{
		std::shared_lock _(m_dataMutex);

		for (auto& [ key, value ] : m_data)
		{
			keys.insert(key);
		}
	}

	for (auto key : keys)
	{
		SendKey(target, key);
	}
}

void StateBagImpl::SendKeyAll(std::string_view key)
{
	if (m_routingTargets.empty())
	{
		auto [lock, refTargets] = m_parent->GetTargets();
		auto targets = refTargets.get();

		for (int target : targets)
		{
			SendKey(target, key);
		}
	}
	else
	{
		std::shared_lock _(m_routingTargetsMutex);

		for (int target : m_routingTargets)
		{
			SendKey(target, key);
		}
	}
}

void StateBagImpl::SendKey(int target, std::string_view key)
{
	static thread_local rl::MessageBuffer dataBuffer(131072);
	dataBuffer.SetCurrentBit(0);

	auto writeStr = [](const auto& str)
	{
		dataBuffer.Write<uint16_t>(16, str.size() + 1);
		dataBuffer.WriteBits(str.data(), str.size() * 8);
		dataBuffer.Write<uint8_t>(8, 0);
	};

	{
		std::shared_lock _(m_dataMutex);

		auto keyDataIt = m_data.find(key);

		if (keyDataIt == m_data.end())
		{
			return;
		}

		auto keyData = keyDataIt->second;

		writeStr(m_id);
		writeStr(key);
		dataBuffer.WriteBits(keyData.data(), keyData.size() * 8);
	}

	m_parent->QueueSend(target, std::string_view{ reinterpret_cast<const char*>(dataBuffer.GetBuffer().data()), dataBuffer.GetCurrentBit() / 8 });
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

std::shared_ptr<StateBag> StateBagComponentImpl::RegisterStateBag(std::string_view id)
{
	auto bag = std::make_shared<StateBagImpl>(this, id);

	{
		std::unique_lock<std::shared_mutex> lock(m_mapMutex);

		if (auto exIt = m_stateBags.find(std::string{ id }); exIt != m_stateBags.end())
		{
			return exIt->second.lock();
		}

		m_stateBags[std::string{ id }] = bag;
	}

	return bag;
}

void StateBagComponentImpl::UnregisterStateBag(std::string_view id)
{
	std::unique_lock<std::shared_mutex> lock(m_mapMutex);
	m_stateBags.erase(std::string{ id });
}

std::shared_ptr<StateBag> StateBagComponentImpl::GetStateBag(std::string_view id)
{
	std::shared_lock<std::shared_mutex> lock(m_mapMutex);
	auto bag = m_stateBags.find(std::string{ id });

	return (bag != m_stateBags.end()) ? bag->second.lock() : nullptr;
}

void StateBagComponentImpl::RegisterTarget(int id)
{
	bool isNew = false;

	{
		std::unique_lock<std::shared_mutex> lock(m_mapMutex);

		if (m_targets.find(id) == m_targets.end())
		{
			isNew = true;
		}

		m_targets.insert(id);
	}

	if (isNew)
	{
		std::shared_lock<std::shared_mutex> lock(m_mapMutex);

		for (auto& data : m_stateBags)
		{
			auto entry = data.second.lock();

			if (entry)
			{
				entry->SendAll(id);
			}
		}
	}
}

void StateBagComponentImpl::UnregisterTarget(int id)
{
	std::unique_lock<std::shared_mutex> lock(m_mapMutex);
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

void StateBagComponentImpl::AttachToObject(fx::ResourceManager* object)
{

}

void StateBagComponentImpl::HandlePacket(int source, std::string_view dataRaw)
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

		return rbuffer;
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

	// read data
	size_t dataLength = (buffer.GetLength() * 8) - buffer.GetCurrentBit();

	std::vector<uint8_t> data(dataLength / 8);
	buffer.ReadBits(data.data(), dataLength);

	// handle data
	auto bag = GetStateBag(std::string_view{ idNameBuffer.data(), idNameBuffer.size() });

	if (bag)
	{
		auto bagRef = std::static_pointer_cast<StateBagImpl>(bag);

		// TODO: rate checks, policy checks
		auto peer = bagRef->GetOwningPeer();

		if (peer)
		{
			if (*peer != source)
			{
				return;
			}
		}

		bagRef->SetKey(
			std::string_view{ keyBuffer.data(), keyBuffer.size() },
			std::string_view{ reinterpret_cast<char*>(data.data()), data.size() },
			m_role == StateBagRole::Server);
	}
}

void StateBagComponentImpl::QueueSend(int target, std::string_view packet)
{
	m_gameInterface->SendPacket(target, packet);
}

fwRefContainer<StateBagComponent> StateBagComponent::Create(StateBagRole role)
{
	return new StateBagComponentImpl(role);
}
}
