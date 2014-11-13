#pragma once

class CEntity
{
private:
public:
	char pad[40];
	uint16_t RandomSeed; // ? // +40
	uint16_t m_nModelIndex; // +42
	char pad2[8]; // +44
	void* m_pInstance; // +52
	char pad3[16]; // +56
	CEntity* m_pLod; // +76

	virtual ~CEntity() = 0;

	virtual void m4() = 0;
	virtual void m8() = 0;
	virtual void mC() = 0;
	virtual void m10() = 0;
	virtual void m14() = 0;
	virtual void Remove() = 0;
	virtual void m1C() = 0;
	virtual void m20() = 0;
	virtual void m24() = 0;
	virtual void m28() = 0;
	virtual void m2C() = 0;
	virtual void m30() = 0;
	virtual void m34() = 0;
	virtual void m38() = 0;
	virtual void m3C() = 0;
	virtual void m40() = 0;
	virtual void DestroyModel() = 0;

	float* GetBoundsHooked(float* out);

	float* GetUnkModelHooked(float* out);
};