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