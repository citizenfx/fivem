/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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