#include <StdInc.h>

#include "ClientRegistry.h"

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
	fx::ClientWeakPtr weakClient(client);

	m_clientsBySlotId[1] = weakClient;

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
	return fx::ClientSharedPtr{};
}

void fx::ClientRegistry::SetHost(const fx::ClientSharedPtr& client)
{
}

void fx::ClientRegistry::AttachToObject(ServerInstanceBase* instance)
{
	m_instance = instance;
}
