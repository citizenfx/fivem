#include <StdInc.h>
#include <ClientDeferral.h>

#include <fxScripting.h>

#include <ResourceManager.h>
#include <ResourceCallbackComponent.h>

#include <MsgpackJson.h>
#include <rapidjson/writer.h>

#include <SharedFunction.h>
#include <UvLoopManager.h>

#include <MonoThreadAttachment.h>

namespace fx
{
ClientDeferral::ClientDeferral(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client)
	: m_client(client), m_instance(instance), m_completed(false)
{
}

ClientDeferral::~ClientDeferral()
{
	if (auto loop = std::move(m_loop); loop.GetRef())
	{
		if (auto kat = std::move(m_keepAliveTimer))
		{
			loop->EnqueueCallback([kat]()
			{
				kat->clear();
				kat->once<uvw::CloseEvent>([kat](const uvw::CloseEvent&, uvw::TimerHandle& h)
				{
					(void)kat;
				});

				kat->close();
			});
		}
	}
}

void ClientDeferral::StartTimer()
{
	using namespace std::chrono_literals;

	auto loop = Instance<net::UvLoopManager>::Get()->GetOrCreate("svMain");
	m_loop = loop;

	auto thisRef = weak_from_this();

	loop->EnqueueCallback([thisRef]()
	{
		if (auto self = thisRef.lock())
		{
			self->StartTimerOnLoopThread();
		}
	});
}

void ClientDeferral::StartTimerOnLoopThread()
{
	auto thisWeak = weak_from_this();

	m_keepAliveTimer = m_loop->Get()->resource<uvw::TimerHandle>();
	m_keepAliveTimer->on<uvw::TimerEvent>([thisWeak](const uvw::TimerEvent&, uvw::TimerHandle& h)
	{
		if (auto self = thisWeak.lock())
		{
			if (auto cb = self->m_messageCallback)
			{
				cb("");
			}
		}
	});

	m_keepAliveTimer->start(1000ms, 1000ms);
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
			auto defRef = ref->deferral.lock();

			if (!defRef || defRef->m_completed)
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

		self->StartTimer();
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
							auto fnRef = functionRef.GetRef();

							gscomms_execute_callback_on_main_thread([self, fnRef, cardJson]
							{
								rapidjson::Document cardJSON;
								cardJSON.Parse(cardJson.c_str(), cardJson.size());

								if (!cardJSON.HasParseError())
								{
									msgpack::object cardObject;
									msgpack::zone zone;
									ConvertToMsgPack(cardJSON, cardObject, zone);

									self->m_instance->GetComponent<fx::ResourceManager>()->CallReference<void>(fnRef, cardObject, cardJson);
								}
							});
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

	cbs["handover"] = cbComponent->CreateCallback(createDeferralCallback([](const std::shared_ptr<ClientDeferral>& self, const std::string& deferralKey, const msgpack::unpacked& unpacked)
	{
		auto obj = unpacked.get().as<std::vector<msgpack::object>>();

		if (obj.size() >= 1)
		{
			try
			{
				auto dict = obj[0].as<std::map<std::string, msgpack::object>>();
				
				for (const auto& [key, value] : dict)
				{
					rapidjson::Document document;
					ConvertToJSON(value, document, document.GetAllocator());

					// write as a json string
					rapidjson::StringBuffer sb;
					rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

					if (document.Accept(writer))
					{
						self->SetHandoverData(key, { sb.GetString(), sb.GetSize() });
					}
				}
			}
			catch (msgpack::type_error& error)
			{
			
			}
		}
	}));

	return cbs;
}
}
