#include <StdInc.h>
#include <ClientDeferral.h>

#include <fxScripting.h>

#include <ResourceManager.h>
#include <ResourceCallbackComponent.h>

namespace fx
{
ClientDeferral::ClientDeferral(fx::ServerInstanceBase* instance, const std::shared_ptr<fx::Client>& client)
	: m_client(client), m_instance(instance), m_completed(false)
{

}

ClientDeferral::~ClientDeferral()
{
	
}

bool ClientDeferral::IsDeferred()
{
	return !m_deferralStates.empty();
}

void ClientDeferral::UpdateDeferrals()
{
	bool allDone = true;
	bool rejected = false;
	std::string rejectionMsg;

	std::stringstream progressMsg;

	for (auto& entry : m_deferralStates)
	{
		auto& ds = entry.second;

		// if not done, append progress
		if (!ds.done)
		{
			progressMsg << fmt::sprintf("%s: %s\n", ds.description, ds.message);
			allDone = false;
		}

		// if rejected, break and set rejection message
		if (ds.rejected)
		{
			rejectionMsg = ds.message;

			rejected = true;
			break;
		}
	}

	if (rejected)
	{
		m_completed = true;

		if (m_rejectCallback)
		{
			m_rejectCallback(rejectionMsg);
		}

		return;
	}

	if (allDone)
	{
		m_completed = true;

		if (m_resolveCallback)
		{
			m_resolveCallback();
		}

		return;
	}

	if (m_messageCallback)
	{
		m_messageCallback(progressMsg.str());
	}
}

TCallbackMap ClientDeferral::GetCallbacks()
{
	TCallbackMap cbs;

	auto resman = m_instance->GetComponent<fx::ResourceManager>();
	auto cbComponent = resman->GetComponent<fx::ResourceCallbackComponent>();

	auto ref = std::make_shared<DeferralCallbackReference>();
	ref->deferral = weak_from_this();

	auto createDeferralCallback = [ref](const auto& cb)
	{
		return [ref, cb](const msgpack::unpacked& unpacked)
		{
			auto& deferral = ref->deferral;

			if (deferral.expired())
			{
				return;
			}

			auto defRef = deferral.lock();

			if (defRef->m_completed)
			{
				return;
			}

			fx::OMPtr<IScriptRuntime> runtime;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					std::string deferralKey = resource->GetName();

					cb(defRef, deferralKey, unpacked);
				}
			}
		};
	};

	cbs["defer"] = cbComponent->CreateCallback(createDeferralCallback([](const std::shared_ptr<ClientDeferral>& self, const std::string& deferralKey, const msgpack::unpacked& unpacked)
	{
		auto ds = DeferralState{};
		ds.description = deferralKey;
		ds.resourceKey = deferralKey;
		ds.message = "Deferring connection...";

		self->m_deferralStates[deferralKey] = std::move(ds);

		self->UpdateDeferrals();
	}));

	cbs["update"] = cbComponent->CreateCallback(createDeferralCallback([](const std::shared_ptr<ClientDeferral>& self, const std::string& deferralKey, const msgpack::unpacked& unpacked)
	{
		auto& deferralState = self->m_deferralStates[deferralKey];

		auto obj = unpacked.get().as<std::vector<msgpack::object>>();

		if (obj.size() == 1)
		{
			deferralState.message = obj[0].as<std::string>();

			self->UpdateDeferrals();
		}
	}));

	cbs["done"] = cbComponent->CreateCallback(createDeferralCallback([](const std::shared_ptr<ClientDeferral>& self, const std::string& deferralKey, const msgpack::unpacked& unpacked)
	{
		auto& deferralState = self->m_deferralStates[deferralKey];
		deferralState.done = true;

		auto obj = unpacked.get().as<std::vector<msgpack::object>>();

		if (obj.size() == 1)
		{
			deferralState.message = obj[0].as<std::string>();
			deferralState.rejected = true;
		}

		self->UpdateDeferrals();
	}));

	return cbs;
}
}