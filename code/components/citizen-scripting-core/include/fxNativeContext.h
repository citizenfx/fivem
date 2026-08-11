/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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