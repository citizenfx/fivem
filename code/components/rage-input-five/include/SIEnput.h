//
// Input library for SIE controllers (Jedi/JediPlus/Bond).
//
#pragma once

namespace io
{
class SIEPad
{
public:
	class Buttons
	{
	public:
		static constexpr inline int Share = (1 << 0);
		static constexpr inline int L3 = (1 << 1);
		static constexpr inline int R3 = (1 << 2);
		static constexpr inline int Options = (1 << 3);
		static constexpr inline int DigitalUp = (1 << 4);
		static constexpr inline int DigitalRight = (1 << 5);
		static constexpr inline int DigitalDown = (1 << 6);
		static constexpr inline int DigitalLeft = (1 << 7);
		static constexpr inline int L2 = (1 << 8);
		static constexpr inline int R2 = (1 << 9);
		static constexpr inline int L1 = (1 << 10);
		static constexpr inline int R1 = (1 << 11);
		static constexpr inline int Triangle = (1 << 12);
		static constexpr inline int Circle = (1 << 13);
		static constexpr inline int Cross = (1 << 14);
		static constexpr inline int Square = (1 << 15);
		static constexpr inline int PS = (1 << 16);
		static constexpr inline int Mute = (1 << 17);
		static constexpr inline int Touchpad = (1 << 20);
	};

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
