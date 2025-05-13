/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

/*
 * Override the value (5th parameter) used in the "ptfx manager init" function to create script ptfx array
 * On b1604 to b3407 default value is 128
 *
 * The game keep track of script ptfxs in an atArray defined as :
 * struct scriptPtfx {
 *     int a;
 *     bool b;
 *     bool c;
 *     char padding[2];
 * };
 *
 * This was tested with a limit up to 1024 without having any obvious bad side effects, but the game doesn't render some fx above ~384?
 * This seems related to PtFxSortedEntity pool, but i didn't manage to resolve the rendering issue
 * I also tried making other parameters of this function to bigger values but it didn't change anything
 */

static HookFunction hookFunction([]
{
  hook::put<int32_t>(hook::get_pattern("C7 44 24 ? ? ? ? ? E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 48 8B CB", 4), 256);
});