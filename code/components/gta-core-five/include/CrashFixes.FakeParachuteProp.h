#pragma once

#include <StdInc.h>
#include "ComponentExport.h"

bool COMPONENT_EXPORT(GTA_CORE_FIVE) IsParachuteModelAuthorized(const uint32_t& modelNameHash);
bool COMPONENT_EXPORT(GTA_CORE_FIVE) IsParachutePackModelAuthorized(const uint32_t& modelNameHash);
void COMPONENT_EXPORT(GTA_CORE_FIVE) AddAuthorizedParachuteModel(const uint32_t& modelNameHash);
void COMPONENT_EXPORT(GTA_CORE_FIVE) AddAuthorizedParachutePackModel(const uint32_t& modelNameHash);
