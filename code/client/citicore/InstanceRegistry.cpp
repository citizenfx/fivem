/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "Registry.h"

// global registry for use by client-only modules
InstanceRegistry g_instanceRegistry;

InstanceRegistry* CoreGetGlobalInstanceRegistry()
{
    return &g_instanceRegistry;
}