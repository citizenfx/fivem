/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
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