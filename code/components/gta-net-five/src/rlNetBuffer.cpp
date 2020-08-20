#include <StdInc.h>
#include <Hooking.h>
#include <MinHook.h>

#include <rlNetBuffer.h>
#include <state/RlMessageBuffer.h>

#include <CrossBuildRuntime.h>
#include <ICoreGameInit.h>

extern ICoreGameInit* icgi;

namespace rl
{
	bool MessageBuffer::GetLengthHackState()
	{
		return icgi->OneSyncBigIdEnabled;
	}
}

static hook::cdecl_stub<bool(void*, uint32_t, int)> _netBuffer_WriteInteger([]()
{
	return hook::get_pattern("48 8B D9 40 84 79 1C 75 6F 8B 49 10", -0x29);
});

static bool(*g_orig_netBuffer_WriteUnsigned)(void* a1, uint32_t a2, int length, int a4);
static bool _netBuffer_WriteUnsigned(void* a1, uint32_t a2, int length, int a4)
{
	if (length == 13 && icgi->OneSyncBigIdEnabled)
	{
		length = 16;
	}

	return g_orig_netBuffer_WriteUnsigned(a1, a2, length, a4);
}

static hook::cdecl_stub<bool(void*, bool)> _netBuffer_WriteBit([]()
{
	return hook::get_pattern("F6 43 1C 01 75 5E", -0xF);
});

static hook::cdecl_stub<bool(void*, uint32_t*, int)> _netBuffer_ReadInteger([]()
{
	return hook::get_pattern("8B 44 24 30 8B D6 48 8B CB 89 07 E8", -0x50);
});

static void(*g_orig_netBuffer_ReadUnsigned)(void* a1, uint32_t* a2, int length, int a4);
static void _netBuffer_ReadUnsigned(void* a1, uint32_t* a2, int length, int a4)
{
	if (length == 13 && icgi->OneSyncBigIdEnabled)
	{
		length = 16;
	}

	return g_orig_netBuffer_ReadUnsigned(a1, a2, length, a4);
}

static void(*g_orig_netBuffer_BumpReadWriteCursor)(rage::datBitBuffer* a1, int length);
static void _netBuffer_BumpReadWriteCursor(rage::datBitBuffer* a1, int length)
{
	if (length == 13 && icgi->OneSyncBigIdEnabled)
	{
		length = 16;
	}

	return g_orig_netBuffer_BumpReadWriteCursor(a1, length);
}

static void(*g_orig_netBuffer_BumpWriteCursor)(rage::datBitBuffer* a1, int length);
static void _netBuffer_BumpWriteCursor(rage::datBitBuffer* a1, int length)
{
	if (length == 13 && icgi->OneSyncBigIdEnabled)
	{
		length = 16;
	}

	return g_orig_netBuffer_BumpWriteCursor(a1, length);
}

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

static HookFunction hookFunction([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("01 51 10 8B 41 10 3B 41 14 7E 03"), _netBuffer_BumpReadWriteCursor, (void**)&g_orig_netBuffer_BumpReadWriteCursor);
	MH_CreateHook(hook::get_pattern("01 51 10 8B 41 10 3B 41 18 7E 03"), _netBuffer_BumpWriteCursor, (void**)&g_orig_netBuffer_BumpWriteCursor);
	MH_CreateHook(hook::get_pattern("41 83 E1 07 45 8B D8 49 C1 FA 03 BB 08 00 00 00", -0x12), _netBuffer_ReadUnsigned, (void**)&g_orig_netBuffer_ReadUnsigned);
	
	// 1868 arxan - maybe?
	MH_CreateHook(hook::get_pattern("49 C1 FA 03 41 83 E1 07 4C 03 D1 B9 20 00 00 00", -0xE), _netBuffer_WriteUnsigned, (void**)&g_orig_netBuffer_WriteUnsigned);

	MH_EnableHook(MH_ALL_HOOKS);
});
