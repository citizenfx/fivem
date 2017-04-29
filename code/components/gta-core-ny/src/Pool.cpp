/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Pool.h"

CPool*& CPools::ms_pBuildingPool = *(CPool**)0x168FED0;
CPool*& CPools::ms_pQuadTreePool = *(CPool**)0x13504D0;

WRAPPER void* CPool::Allocate() { EAXJMP(0x96D520); }