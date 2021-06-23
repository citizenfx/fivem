#pragma once

namespace rage
{
class netInterface_queryFunctions
{
public:
	virtual ~netInterface_queryFunctions() = 0;

	virtual void m_8() = 0;

	virtual void m_10() = 0;

	virtual void m_18() = 0;

	virtual void m_20() = 0;

	virtual void m_28() = 0;

	virtual void m_30() = 0;

	virtual void m_38() = 0;

	virtual void m_40() = 0;

	virtual void m_48() = 0;

	virtual void m_50() = 0;

	virtual void m_58() = 0;

	virtual void m_60() = 0;

	virtual uint32_t GetTimestamp() = 0;

	virtual void m_70() = 0;

	virtual void m_78() = 0;

	virtual void m_80() = 0;

	virtual void m_88() = 0;

public:
	static netInterface_queryFunctions* GetInstance();
};
}
