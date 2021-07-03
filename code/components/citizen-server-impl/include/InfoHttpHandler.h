#pragma once

namespace fx
{
class ServerInstanceBase;

class InfoHttpHandlerComponent : public fwRefCountable, public fx::IAttached<fx::ServerInstanceBase>
{
public:
	virtual void AttachToObject(fx::ServerInstanceBase* instance);

	void GetJsonData(nlohmann::json* infoJson, nlohmann::json* dynamicJson, nlohmann::json* playersJson);

private:
	fwRefContainer<fwRefCountable> m_impl;
};
}

DECLARE_INSTANCE_TYPE(fx::InfoHttpHandlerComponent);
