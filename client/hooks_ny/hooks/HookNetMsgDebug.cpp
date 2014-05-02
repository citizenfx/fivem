#include "StdInc.h"

#if 0
void injectFunction(DWORD dwAddress, DWORD pfnReplacement);

static DWORD netMsgTypeHookCont = 0x4406F7;

struct AutoIdDescriptor
{
	int vtable;
	int pad;
	int id;
	int pad2;
	const char* name;
	int pad3;
	AutoIdDescriptor* next;
};

struct AutoIdDescriptorBase
{
	AutoIdDescriptor* next;
};

static AutoIdDescriptorBase& msgAutoId = *(AutoIdDescriptorBase*)0x19AC90C;

void NetMsgLogType(int* netType)
{
	for (AutoIdDescriptor* descriptor = msgAutoId.next; descriptor->next; descriptor = descriptor->next)
	{
		if (descriptor->id == *netType)
		{
			trace("[%s] %d\n", descriptor->name, *netType);
			return;
		}
	}
}

void __declspec(naked) netMsgTypeHookRetn()
{
	__asm
	{
		push eax
		push ecx
		push edi
		call NetMsgLogType
		pop edi
		pop ecx
		pop eax

		add esp, 4h
		retn
	}
}

void __declspec(naked) netMsgTypeHook()
{
	__asm
	{
		mov eax, [esp + 4]
		push eax
		push offset netMsgTypeHookRetn

		sub esp, 1Ch
		mov al, [esp + 18h]

		jmp netMsgTypeHookCont
	}
}

static HookFunction hookFunction([] ()
{
	injectFunction(0x4406F0, (DWORD)netMsgTypeHook);
});
#endif