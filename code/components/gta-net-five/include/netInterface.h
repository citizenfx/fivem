#pragma once

namespace rage
{
class netInterface_queryFunctions
{
public:
	virtual ~netInterface_queryFunctions() = 0;

	virtual void m_8() = 0;

	virtual uint32_t GetTimestamp() = 0;

public:
	static netInterface_queryFunctions* GetInstance();
};
}
