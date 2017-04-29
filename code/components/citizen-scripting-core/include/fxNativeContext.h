/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

struct fxNativeContext
{
	uintptr_t arguments[32];
	int numArguments;
	int numResults;

	uint64_t nativeIdentifier;
};