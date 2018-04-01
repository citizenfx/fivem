#pragma once

namespace rage
{
class netBlender
{
public:
	virtual ~netBlender() = 0;

	virtual void m_8() = 0;

	virtual void m_10() = 0;

	virtual void m_18() = 0;

	virtual void m_20() = 0;

	virtual void m_28() = 0;

	virtual void m_30() = 0;

	virtual void m_38() = 0;

	virtual void ApplyBlend() = 0;

	virtual void m_48() = 0;

	virtual void m_50() = 0;

	virtual void m_58() = 0;

public:
	void SetTimestamp(uint32_t timestamp);
};
}
