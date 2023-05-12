#pragma once

#include <map>
#include <set>
#include <shared_mutex>

#include "ComponentExport.h"
#include "ComponentHolder.h"

namespace fx
{
class Resource;
class ResourceManager;

namespace resources
{
enum ScanMessageType
{
	Info,
	Warning,
	Error,
};

// ScanMessage represents a single message in a ScanResult
struct COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) ScanMessage
{
	// type is the severity of the message
	ScanMessageType type = ScanMessageType::Info;

	// resource is the resource name affected by the message
	std::string resource;

	// identifier is a code-friendly identifier of the message
	std::string identifier;

	// args contains formatted arguments for the message
	std::vector<std::string> args;

	// Format makes a human-readable string from the message
	std::string Format() const;
};

// ScanResult is the output data from ScanResources
struct COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) ScanResult
{
	// newResources is the number of resources that were new during this scan
	int newResources = 0;

	// updatedResources is the number of resources that were updated during this scan
	int updatedResources = 0;

	// reloadedResources is the number of resources that were reloaded during this scan
	int reloadedResources = 0;

	// resources contains a list of *new* resources discovered during this scan
	std::vector<fwRefContainer<Resource>> resources;

	// messages contains a list of messages encountered during this scan
	std::vector<ScanMessage> messages;
};

// ServerResourceList handles server resource scanning behavior for a ResourceManager
class ServerResourceList : public fwRefCountable, public fx::IAttached<fx::ResourceManager>
{
public:
	// Create is an explicit export to work around DLL linkage issues
	static COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) fwRefContainer<ServerResourceList> Create();

	// ScanResources reads resources from the specified root
	void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) ScanResources(const std::string& resourcePath, ScanResult* outResult = nullptr);

	// AddError adds an error to the current scan result
	void AddError(ScanMessageType type, const std::string& resource, const std::string& identifier, std::vector<std::string>&& args);

	// FindByPathComponent returns all resource names for the specified path component (such as a category)
	std::set<std::string> FindByPathComponent(const std::string& pathComponent);

	// AttachToObject is an implementation of IAttached
	virtual void AttachToObject(fx::ResourceManager* object) override;

private:
	std::shared_mutex m_resourcesByComponentMutex;
	std::map<std::string, std::set<std::string>> m_resourcesByComponent;

	std::map<std::string, std::string> m_scanData;

	std::mutex m_resultMutex;
	ScanResult* m_currentResult = nullptr;

	fx::ResourceManager* m_manager = nullptr;
};
}
}

DECLARE_INSTANCE_TYPE(fx::resources::ServerResourceList);
