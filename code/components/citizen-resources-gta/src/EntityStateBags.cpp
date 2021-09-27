#include <StdInc.h>

#if __has_include(<EntitySystem.h>) && defined(GTA_FIVE)
#include <ResourceManager.h>
#include <StateBagComponent.h>
#include <EntitySystem.h>

#include <ScriptEngine.h>

class CEntityLocalStateBagExtension : public rage::fwExtension
{
public:
	virtual ~CEntityLocalStateBagExtension() = default;

	void Update(int scriptGuid)
	{
		if (m_scriptGuid != scriptGuid)
		{
			auto sbac = fx::ResourceManager::GetCurrent()->GetComponent<fx::StateBagComponent>();
			m_scriptGuid = scriptGuid;
			m_stateBag = sbac->RegisterStateBag(fmt::sprintf("localEntity:%d", scriptGuid));
		}
	}

	virtual int GetExtensionId() const override
	{
		return GetClassId();
	}

	static int GetClassId()
	{
		return (int)96;
	}

private:
	int m_scriptGuid = 0;
	std::shared_ptr<fx::StateBag> m_stateBag;
};

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("ENSURE_ENTITY_STATE_BAG", [](fx::ScriptContext& context)
	{
		auto sguid = context.GetArgument<int>(0);
		if (auto entity = rage::fwScriptGuid::GetBaseFromGuid(sguid))
		{
			auto extension = entity->GetExtension<CEntityLocalStateBagExtension>();

			if (!extension)
			{
				extension = new CEntityLocalStateBagExtension();
				entity->AddExtension(extension);
			}

			extension->Update(sguid);
		}
	});
});
#endif
