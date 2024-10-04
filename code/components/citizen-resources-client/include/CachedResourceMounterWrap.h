#pragma once

#include "ComponentExport.h"
#include "ComponentHolder.h"

namespace fx
{
class Resource;
class CachedResourceMounter;

class COMPONENT_EXPORT(CITIZEN_RESOURCES_CLIENT) CachedResourceMounterWrap : public fwRefCountable, public fx::IAttached<fx::Resource>
{
public:
	explicit CachedResourceMounterWrap(CachedResourceMounter* mounter);

	virtual void AttachToObject(fx::Resource* resource) override;

	bool MountOverlay(const std::string& overlayName, std::string* outError);

private:
	fx::Resource* m_resource;
	CachedResourceMounter* m_mounter;
};
}

DECLARE_INSTANCE_TYPE(fx::CachedResourceMounterWrap);
