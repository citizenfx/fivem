#include "StdInc.h"
#include "EventCore.h"

void fwRefCountable::AddRef()
{
	InterlockedIncrement(&m_refCount.GetCount());
}

void fwRefCountable::Release()
{
	uint32_t c = InterlockedDecrement(&m_refCount.GetCount());

	if (c == 0)
	{
		delete this;
	}
}

fwRefCountable::~fwRefCountable()
{

}