#pragma once

#include <regex>
#include <optional>
#include <functional>
#include <shared_mutex>
#include <atomic>

#include <Resource.h>

#include <rapidjson/document.h>

namespace fx
{
class ResourceConfigurationCacheComponent : public fwRefCountable, public IAttached<Resource>
{
	Resource* m_resource{};
	std::atomic<bool> m_dirty{true};
	rapidjson::Document m_configuration{};
	std::shared_mutex m_lock{};

public:
	void GetConfiguration(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator,
	                      const std::function<std::optional<std::string>(const std::string&)>& getFileServer);

	void Invalidate();

	void AttachToObject(Resource* object) override;
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceConfigurationCacheComponent);
