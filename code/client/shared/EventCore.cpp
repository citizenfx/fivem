/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "EventCore.h"

void fwRefCountable::AddRef()
{
	m_refCount.GetCount()++;
}

bool fwRefCountable::Release()
{
	uint32_t c = m_refCount.GetCount().fetch_sub(1);

	if (c <= 1)
	{
		delete this;
		return true;
	}

	return false;
}

fwRefCountable::~fwRefCountable()
{

}

// here temporarily, hopefully
class InitIoBuf
{
public:
	InitIoBuf()
	{
		setvbuf(stdout, nullptr, _IONBF, 0);
		setvbuf(stderr, nullptr, _IONBF, 0);
	}
};

InitIoBuf initIoBuf;