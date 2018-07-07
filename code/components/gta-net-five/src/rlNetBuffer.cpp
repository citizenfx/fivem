#include <StdInc.h>
#include <Hooking.h>

#include <rlNetBuffer.h>

static hook::cdecl_stub<bool(void*, uint32_t, int)> _netBuffer_WriteInteger([]()
{
	return hook::get_pattern("48 8B D9 40 84 79 1C 75 6F 8B 49 10", -0x29);
});

static hook::cdecl_stub<bool(void*, bool)> _netBuffer_WriteBit([]()
{
	return hook::get_pattern("F6 43 1C 01 75 5E", -0xF);
});

static hook::cdecl_stub<bool(void*, uint32_t*, int)> _netBuffer_ReadInteger([]()
{
	return hook::get_pattern("2D 44 8B 4B 08 48 8B 0B 48 8D 54 24 30", -0x38);
});

namespace rage
{
bool netBuffer::ReadInteger(uint32_t* integer, int bits)
{
	return _netBuffer_ReadInteger(this, integer, bits);
}

bool netBuffer::WriteInteger(uint32_t integer, int bits)
{
	return _netBuffer_WriteInteger(this, integer, bits);
}

bool netBuffer::WriteBit(bool bit)
{
	return _netBuffer_WriteBit(this, bit);
}
}
