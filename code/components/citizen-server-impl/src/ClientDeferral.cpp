#include <StdInc.h>
#include <ClientDeferral.h>

#include <fxScripting.h>

#include <ResourceManager.h>
#include <ResourceCallbackComponent.h>

#include <MsgpackJson.h>
#include <rapidjson/writer.h>

#include <MonoThreadAttachment.h>

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

void ClientDeferral::PresentCard(const std::string& cardJson)
{
	if (m_cardCallback)
	{
		m_cardCallback(cardJson);
	}
}

void ClientDeferral::HandleCardResponse(const std::string& dataJson)
{
	if (m_cardResponseCallback)
	{
		m_cardResponseCallback(dataJson);
	}
}

// blindly copypasted from StackOverflow (to allow std::function to store the funcref types with their move semantics)
// TODO: we use this *thrice* now, time for a shared header?
template<class F>
struct shared_function
{
	std::shared_ptr<F> f;
	shared_function() = default;
	shared_function(F&& f_) : f(std::make_shared<F>(std::move(f_))) {}
	shared_function(shared_function const&) = default;
	shared_function(shared_function&&) = default;
	shared_function& operator=(shared_function const&) = default;
	shared_function& operator=(shared_function&&) = default;

	template<class...As>
	auto operator()(As&& ...as) const
	{
		return (*f)(std::forward<As>(as)...);
	}
};

template<class F>
shared_function<std::decay_t<F>> make_shared_function(F&& f)
{
	return { std::forward<F>(f) };
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

	cbs["presentCard"] = cbComponent->CreateCallback(createDeferralCallback([](const std::shared_ptr<ClientDeferral>& self, const std::string& deferralKey, const msgpack::unpacked& unpacked)
	{
		auto& deferralState = self->m_deferralStates[deferralKey];

		auto obj = unpacked.get().as<std::vector<msgpack::object>>();

		if (obj.size() >= 1)
		{
			if (obj[0].type == msgpack::type::STR || obj[0].type == msgpack::type::BIN)
			{
				self->PresentCard(obj[0].as<std::string>());
			}
			else
			{
				rapidjson::Document document;
				ConvertToJSON(obj[0], document, document.GetAllocator());

				// write as a json string
				rapidjson::StringBuffer sb;
				rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

				if (document.Accept(writer))
				{
					self->PresentCard({ sb.GetString(), sb.GetSize() });
				}
			}

			if (obj.size() >= 2)
			{
				const auto& callback = obj[1];

				if (callback.type == msgpack::type::EXT)
				{
					if (callback.via.ext.type() == 10 || callback.via.ext.type() == 11)
					{
						fx::FunctionRef functionRef{ std::string{callback.via.ext.data(), callback.via.ext.size} };

						self->SetCardResponseHandler(make_shared_function([self, functionRef = std::move(functionRef)](const std::string& cardJson)
						{
							rapidjson::Document cardJSON;
							cardJSON.Parse(cardJson.c_str(), cardJson.size());

							if (!cardJSON.HasParseError())
							{
								msgpack::object cardObject;
								msgpack::zone zone;
								ConvertToMsgPack(cardJSON, cardObject, zone);

								// make sure the monkeys are happy
								MonoEnsureThreadAttached();

								self->m_instance->GetComponent<fx::ResourceManager>()->CallReference<void>(functionRef.GetRef(), cardObject, cardJson);
							}
						}));
					}
				}
			}
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
