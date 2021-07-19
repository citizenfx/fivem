//
// Input library for SIE controllers (Jedi/JediPlus/Bond).
//
#pragma once

namespace io
{
class SIEPad
{
public:
	virtual uint32_t GetButtons() = 0;

	virtual void SetLightBar(uint8_t r, uint8_t g, uint8_t b) = 0;

	virtual void SetVibration(uint8_t l, uint8_t r) = 0;
};

class SIEPadManager
{
public:
	virtual int GetNumPads() = 0;

	virtual std::shared_ptr<SIEPad> GetPad(int index) = 0;
};
}
