/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

// Core object model base interface definition.
class fxIBase
{
public:
	virtual result_t QueryInterface(const guid_t& riid, void** outObject) = 0;

	virtual uint32_t AddRef() = 0;

	virtual uint32_t Release() = 0;
};