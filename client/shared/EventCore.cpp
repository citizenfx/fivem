/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "EventCore.h"

void fwRefCountable::AddRef()
{
	InterlockedIncrement(&m_refCount.GetCount());
}

bool fwRefCountable::Release()
{
	uint32_t c = InterlockedDecrement(&m_refCount.GetCount());

	if (c == 0)
	{
		delete this;
		return true;
	}

	return false;
}

fwRefCountable::~fwRefCountable()
{

}