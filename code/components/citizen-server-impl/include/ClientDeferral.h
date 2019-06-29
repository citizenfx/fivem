#pragma once

#include <map>
#include <memory>

#include <Client.h>
#include <ResourceCallbackComponent.h>

#include <ServerInstanceBase.h>

namespace fx
{
using TCallbackMap = std::map<std::string, fx::ResourceCallbackComponent::CallbackRef>;

class ClientDeferral : public std::enable_shared_from_this<ClientDeferral>
{
public:
	using TCardCallback = std::function<void(const std::string&)>;
	using TMessageCallback = std::function<void(const std::string&)>;
	using TResolveCallback = std::function<void()>;
	using TRejectCallback = std::function<void(const std::string&)>;

public:
	ClientDeferral(fx::ServerInstanceBase* instance, const std::shared_ptr<fx::Client>& client);

	virtual ~ClientDeferral();

	inline void SetResolveCallback(const TResolveCallback& callback)
	{
		m_resolveCallback = callback;
	}

	inline void SetRejectCallback(const TRejectCallback& callback)
	{
		m_rejectCallback = callback;
	}

	inline void SetMessageCallback(const TMessageCallback& callback)
	{
		m_messageCallback = callback;
	}

	inline void SetCardCallback(const TCardCallback& callback)
	{
		m_cardCallback = callback;
	}

	inline void SetCardResponseHandler(const TCardCallback& callback)
	{
		m_cardResponseCallback = callback;
	}

	void PresentCard(const std::string& cardJson);

	void HandleCardResponse(const std::string& dataJson);

	bool IsDeferred();

	void UpdateDeferrals();

	TCallbackMap GetCallbacks();

private:
	struct DeferralCallbackReference
	{
		std::weak_ptr<ClientDeferral> deferral;
	};

	struct DeferralState
	{
		bool done;
		bool rejected;

		std::string resourceKey;
		std::string description;
		std::string message;

		inline DeferralState()
			: done(false), rejected(false)
		{

		}
	};

private:
	TResolveCallback m_resolveCallback;
	TRejectCallback m_rejectCallback;
	TMessageCallback m_messageCallback;
	TCardCallback m_cardCallback;
	TCardCallback m_cardResponseCallback;

	std::weak_ptr<fx::Client> m_client;

	std::map<std::string, DeferralState> m_deferralStates;

	bool m_completed;

	fx::ServerInstanceBase* m_instance;
};
}
