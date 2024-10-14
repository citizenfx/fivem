#include <StdInc.h>

#include "ClientRegistry.h"

#include "GameServer.h"

struct FarLockImpl : fx::FarLock::ImplBase
{
	// actual implementation uses folly's shared mutex
	std::shared_mutex mutex;

	bool TryLock() override
	{
		return mutex.try_lock();
	}

	void LockShared() override
	{
		return mutex.lock_shared();
	}

	void Lock() override
	{
		return mutex.lock();
	}

	void Unlock() override
	{
		return mutex.unlock();
	}

	void UnlockShared() override
	{
		return mutex.unlock_shared();
	}
};

fx::FarLock::FarLock()
	: impl(std::make_unique<FarLockImpl>())
{
}

fx::FarLock::~FarLock()
{
}

fx::ClientRegistry::ClientRegistry()
: m_hostNetId(-1), m_curNetId(1), m_instance(nullptr)
{
	m_clientsBySlotId.resize(MAX_CLIENTS);
}

fx::ClientSharedPtr fx::ClientRegistry::MakeClient(const std::string& guid)
{
	fx::ClientSharedPtr client = fx::ClientSharedPtr::Construct(guid);
	client->SetNetId(m_amountConnectedClients + 1);

	const auto local = net::PeerAddress::FromString("127.0.0.1").get();
	client->SetPeer(1, local);

	client->SetName(guid);

	fx::ClientWeakPtr weakClient(client);

	m_clientsBySlotId[client->GetNetId()] = weakClient;
	m_clientsByNetId[client->GetNetId()] = weakClient;
	
	{
		std::unique_lock writeHolder(m_clientMutex);
		m_clients.emplace(guid, client);
	}

	++m_amountConnectedClients;

	return client;
}

void fx::ClientRegistry::HandleConnectingClient(const fx::ClientSharedPtr& client)
{
}

void fx::ClientRegistry::HandleConnectedClient(const fx::ClientSharedPtr& client, uint32_t oldNetID)
{
}

fx::ClientSharedPtr fx::ClientRegistry::GetHost()
{
	if (m_hostNetId == 0xFFFF)
	{
		return fx::ClientSharedPtr{};
	}

	return GetClientByNetID(m_hostNetId);
}

void fx::ClientRegistry::SetHost(const fx::ClientSharedPtr& client)
{
	if (!client)
	{
		m_hostNetId = -1;
	}
	else if (!fx::IsOneSync())
	{
		m_hostNetId = client->GetNetId();
	}
}

void fx::ClientRegistry::AttachToObject(ServerInstanceBase* instance)
{
	m_instance = instance;
}
