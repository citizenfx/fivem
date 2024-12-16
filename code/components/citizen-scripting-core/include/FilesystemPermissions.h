#pragma once

#include "ComponentExport.h"

namespace fx
{
COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
bool ScriptingFilesystemAllowWrite(const std::string& path);
}
