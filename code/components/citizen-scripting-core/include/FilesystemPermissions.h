#pragma once

#include "ComponentExport.h"


namespace fx
{
class Resource;

COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
bool ScriptingFilesystemAllowWrite(const std::string& path, fx::Resource* resourceOverride = nullptr);
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
bool ScriptingWorkerAllowSpawn(fx::Resource* resourceOverride = nullptr);
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
bool ScriptingChildProcessAllowSpawn(fx::Resource* resourceOverride = nullptr);
}
