#include <StdInc.h>
#include <Hooking.h>

#include <rlNetBuffer.h>

static hook::cdecl_stub<bool(void*, uint32_t, int)> _netBuffer_WriteInteger([]()
{
	return hook::get_pattern("48 8B D9 40 84 79 1C 75 6F 8B 49 10", -0x29);
});

static hook::cdecl_stub<void(void*, uint32_t, int, int)> _netBuffer_WriteUnsigned([]()
{
	return hook::get_pattern("49 C1 FA 03 41 83 E1 07 4C 03  D1 B9 20 00 00 00", -0xE);
});

static hook::cdecl_stub<bool(void*, bool)> _netBuffer_WriteBit([]()
{
	return hook::get_pattern("F6 43 1C 01 75 5E", -0xF);
});

static hook::cdecl_stub<bool(void*, uint32_t*, int)> _netBuffer_ReadInteger([]()
{
	return hook::get_pattern("8B 44 24 30 8B D6 48 8B CB 89 07 E8", -0x50);
});

static hook::cdecl_stub<void(rage::datBitBuffer*, int)> _netBuffer_BumpReadWriteCursor([]()
{
	return hook::get_pattern("01 51 10 8B 41 10 3B 41 14 7E 03");
});

namespace rage
{
bool datBitBuffer::ReadInteger(uint32_t* integer, int bits)
{
	return _netBuffer_ReadInteger(this, integer, bits);
}

bool datBitBuffer::WriteInteger(uint32_t integer, int bits)
{
	return _netBuffer_WriteInteger(this, integer, bits);
}

bool datBitBuffer::WriteUns(uint32_t integer, int bits)
{
	// this method only exists inlined in GTA V so we have to manually implement it
	if (m_f1C & 1)
	{
		return false;
	}

	if (m_unkBit + bits > m_maxBit)
	{
		return false;
	}

	if (!(m_f1C & 2))
	{
		_netBuffer_WriteUnsigned(m_data, integer, bits, m_f8 + m_unkBit);
	}

	_netBuffer_BumpReadWriteCursor(this, bits);

	return true;
}

bool datBitBuffer::WriteBit(bool bit)
{
	return _netBuffer_WriteBit(this, bit);
}
}
