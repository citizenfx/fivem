#pragma once

namespace rage
{
class netBlender
{
public:
	virtual ~netBlender() = 0;

	virtual void m_8() = 0; // IsBlendingOn

	virtual void m_10() = 0; // GetBlenderData

	virtual void m_18() = 0; // GetBlenderData

	virtual void SetTimestamp(uint32_t timestamp) = 0; // 20 SetLastSyncMessageTime

	virtual void m_40() = 0;

	virtual void m_30() = 0;

	virtual void m_28() = 0; // 38 OnOwnerChange

	virtual void m_48() = 0;
	
	virtual void m_50() = 0;

	virtual void m_68() = 0;

	virtual void m_38() = 0; // 58 Reset

	virtual void ApplyBlend() = 0; // 60 GoStraightToTarget

	virtual void m_70() = 0;

	virtual void m_78() = 0;

	virtual void m_58() = 0; // ProcessPostPhysics

	virtual void m_80() = 0;

	virtual void m_88() = 0;
};
}
