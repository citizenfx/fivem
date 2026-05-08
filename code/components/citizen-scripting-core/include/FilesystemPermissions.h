#pragma once

#include "ComponentExport.h"

#include <cstdint>
#include <string>
#include <vector>


namespace fx
{
class Resource;

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
bool ScriptingFilesystemAllowWrite(const std::string& path, fx::Resource* resourceOverride = nullptr);
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
uint64_t ScriptingFilesystemPushBuildTaskPermission(const std::string& sourceResource, const std::string& targetResource, const std::vector<std::string>& allowedPaths);
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
void ScriptingFilesystemPopBuildTaskPermission(uint64_t permissionId);
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
bool ScriptingFilesystemIsBuildTaskPathSafe(const std::string& path);
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
bool ScriptingWorkerAllowSpawn(fx::Resource* resourceOverride = nullptr);
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
bool ScriptingChildProcessAllowSpawn(fx::Resource* resourceOverride = nullptr);
}
