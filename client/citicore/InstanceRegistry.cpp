/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Registry.h"

// global registry for use by client-only modules
InstanceRegistry g_instanceRegistry;

InstanceRegistry* CoreGetGlobalInstanceRegistry()
{
    return &g_instanceRegistry;
}