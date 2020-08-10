#include <StdInc.h>

#include <MakeClientFunction.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <ResourceManager.h>
#include <ScriptEngine.h>

#include <HttpClient.h>

#include <ServerLicensingComponent.h>

#include <json.hpp>

#include <optional>

inline std::string GetLicenseKey()
{
	auto resourceManager = fx::ResourceManager::GetCurrent();
	auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

	auto var = instance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("sv_licenseKeyToken");

	if (!var || var->GetValue().empty())
	{
		return "";
	}

	auto licensing = instance->GetComponent<ServerLicensingComponent>();

	return licensing->GetLicenseKey();
}

class CommerceComponent : public fwRefCountable
{
public:
	CommerceComponent(fx::ClientWeakPtr client)
		: m_commerceDataLoaded(false), m_client(client)
	{
		
	}

	inline bool HasCommerceDataLoaded()
	{
		return m_commerceDataLoaded;
	}

	void LoadCommerceData();

	std::optional<int> GetUserId();

	void SetSkus(std::set<int>&& list);

	bool OwnsSku(int sku);

	void RequestSkuPurchase(int sku);

private:
	fx::ClientWeakPtr m_client;

	bool m_commerceDataLoaded;

	std::set<int> m_ownedSkus;
};

static HttpClient* httpClient;

void CommerceComponent::LoadCommerceData()
{
	auto userId = GetUserId();

	if (m_commerceDataLoaded || !userId)
	{
		return;
	}

	fwRefContainer<CommerceComponent> thisRef(this);

	httpClient->DoGetRequest(fmt::sprintf(LICENSING_EP "api/entitlements/%d/%s", *userId, GetLicenseKey()), [thisRef](bool success, const char* data, size_t length)
	{
		if (success)
		{
			try
			{
				auto json = nlohmann::json::parse(std::string(data, length));
				std::set<int> skuIds;

				for (auto& entry : json["entitlements"])
				{
					skuIds.insert(entry.value<int>("sku_id", 0));
				}

				thisRef->SetSkus(std::move(skuIds));
			}
			catch (const std::exception& e)
			{

			}
		}
	});
}

void CommerceComponent::SetSkus(std::set<int>&& list)
{
	m_ownedSkus = std::move(list);
	m_commerceDataLoaded = true;
}

bool CommerceComponent::OwnsSku(int sku)
{
	return m_ownedSkus.find(sku) != m_ownedSkus.end();
}

void CommerceComponent::RequestSkuPurchase(int sku)
{
	auto userId = GetUserId();

	if (!userId)
	{
		return;
	}

	fwRefContainer<CommerceComponent> thisRef(this);
	auto clientRef = m_client.lock();

	httpClient->DoGetRequest(fmt::sprintf(LICENSING_EP "api/paymentRequest/%d/%d/%s", *userId, sku, GetLicenseKey()), [thisRef, clientRef](bool success, const char* data, size_t length)
	{
		if (success)
		{
			// build the target event
			net::Buffer outBuffer;
			outBuffer.Write(HashRageString("msgPaymentRequest"));

			// payload
			outBuffer.Write(data, length);

			// send along
			clientRef->SendPacket(0, outBuffer, NetPacketType_Reliable);
		}
	});
}

std::optional<int> CommerceComponent::GetUserId()
{
	auto clientRef = m_client.lock();
	const auto& identifiers = clientRef->GetIdentifiers();

	for (const auto& identifier : identifiers)
	{
		if (identifier.find("fivem:") == 0)
		{
			int userId = atoi(identifier.substr(6).c_str());

			if (userId != 0)
			{
				return userId;
			}
		}
	}

	return {};
}

DECLARE_INSTANCE_TYPE(CommerceComponent);

static InitFunction initFunction([]()
{
	httpClient = new HttpClient(L"FXServer/Licensing");

	fx::ScriptEngine::RegisterNativeHandler("CAN_PLAYER_START_COMMERCE_SESSION", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> uint32_t
	{
		return client->GetComponent<CommerceComponent>()->GetUserId() ? true : false;
	}));

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

		clientRegistry->OnClientCreated.Connect([=](const fx::ClientSharedPtr& client)
		{
			client->SetComponent(new CommerceComponent(client));
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("LOAD_PLAYER_COMMERCE_DATA", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> uint32_t
	{
		auto commerceData = client->GetComponent<CommerceComponent>();

		commerceData->LoadCommerceData();

		return commerceData->HasCommerceDataLoaded();
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_PLAYER_COMMERCE_INFO_LOADED", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> uint32_t
	{
		auto commerceData = client->GetComponent<CommerceComponent>();

		return commerceData->HasCommerceDataLoaded();
	}));

	fx::ScriptEngine::RegisterNativeHandler("DOES_PLAYER_OWN_SKU", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> uint32_t
	{
		auto commerceData = client->GetComponent<CommerceComponent>();

		return commerceData->OwnsSku(context.GetArgument<int>(1));
	}));

	fx::ScriptEngine::RegisterNativeHandler("REQUEST_PLAYER_COMMERCE_SESSION", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> bool
	{
		auto commerceData = client->GetComponent<CommerceComponent>();
		commerceData->RequestSkuPurchase(context.GetArgument<int>(1));

		return true;
	}));
});
